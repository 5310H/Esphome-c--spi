#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include <sodium.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "api_message.pb.h"
#include "api_encryption.pb.h"
#include "api_service.pb.h"

/* ---------------------------------------------------------
 * ESPHome message type IDs (official)
 * --------------------------------------------------------- */
enum {
    MSG_HELLO_REQUEST        = 1,
    MSG_HELLO_RESPONSE       = 2,
    MSG_ENCRYPTION_REQUEST   = 3,
    MSG_ENCRYPTION_RESPONSE  = 4,
    MSG_ENCRYPTED_MESSAGE    = 5,
    MSG_CONNECT_REQUEST      = 6,
    MSG_CONNECT_RESPONSE     = 7,
    MSG_SWITCH_STATE_RESPONSE = 17,
    MSG_PING_REQUEST         = 9,
    MSG_PING_RESPONSE        = 10,
    MSG_LIST_ENTITIES_REQUEST = 11,
    MSG_LIST_ENTITIES_SENSOR_RESPONSE = 12,
    MSG_LIST_ENTITIES_BINARY_SENSOR_RESPONSE = 13,
    MSG_LIST_ENTITIES_SWITCH_RESPONSE = 14,
    MSG_LIST_ENTITIES_LIGHT_RESPONSE = 15,
    MSG_LIST_ENTITIES_DONE_RESPONSE = 19,
    MSG_SUBSCRIBE_STATES_REQUEST = 20,
    MSG_SENSOR_STATE_RESPONSE = 21,
    MSG_BINARY_SENSOR_STATE_RESPONSE = 22,
    MSG_SWITCH_COMMAND_REQUEST = 26
};

/* ---------------------------------------------------------
 * Send raw protobuf message with 4-byte type + 2-byte length
 * --------------------------------------------------------- */
static bool send_frame(int sock, uint32_t type,
                       const uint8_t *payload, size_t len)
{
    if (len > 0xFFFF) {
        fprintf(stderr, "Payload too large for 16-bit length field\n");
        return false;
    }

    uint8_t header[6];
    header[0] = (type >> 24) & 0xFF;
    header[1] = (type >> 16) & 0xFF;
    header[2] = (type >> 8)  & 0xFF;
    header[3] = (type)       & 0xFF;
    header[4] = (len >> 8) & 0xFF;
    header[5] = (len)      & 0xFF;

    size_t total_sent = 0;
    while (total_sent < 6) {
        ssize_t n = write(sock, header + total_sent, 6 - total_sent);
        if (n <= 0) return false;
        total_sent += n;
    }

    total_sent = 0;
    while (total_sent < len) {
        ssize_t n = write(sock, payload + total_sent, len - total_sent);
        if (n <= 0) return false;
        total_sent += n;
    }

    return true;
}

/* ---------------------------------------------------------
 * Receive raw protobuf message
 * --------------------------------------------------------- */
static bool recv_frame(int sock, uint32_t *type,
                       uint8_t *payload, size_t max_len, size_t *len)
{
    uint8_t header[6];
    size_t total_read = 0;

    while (total_read < 6) {
        ssize_t n = read(sock, header + total_read, 6 - total_read);
        if (n <= 0) return false;
        total_read += n;
    }

    *type = (header[0] << 24) |
            (header[1] << 16) |
            (header[2] << 8)  |
            (header[3]);

    *len = (header[4] << 8) | header[5];

    if (*len > max_len) {
        fprintf(stderr, "Received frame too large (%zu > %zu)\n", *len, max_len);
        return false;
    }

    total_read = 0;
    while (total_read < *len) {
        ssize_t n = read(sock, payload + total_read, *len - total_read);
        if (n <= 0) return false;
        total_read += n;
    }

    return true;
}

/* ---------------------------------------------------------
 * Encode a protobuf message into a buffer
 * --------------------------------------------------------- */
static size_t encode_message(const pb_field_t fields[],
                             const void *msg,
                             uint8_t *out)
{
    pb_ostream_t stream = pb_ostream_from_buffer(out, 2048);
    if (!pb_encode(&stream, fields, msg)) {
        printf("encode failed\n");
        return 0;
    }
    return stream.bytes_written;
}

/* ---------------------------------------------------------
 * Nanopb callback for encoding bytes/strings
 * --------------------------------------------------------- */
typedef struct {
    const uint8_t *buffer;
    size_t length;
} pb_arg_t;

static bool encode_bytes_cb(void *stream, const pb_field_t *field, void *const *arg)
{
    pb_ostream_t *os = (pb_ostream_t *)stream;
    /* arg points to the field 'arg' in the pb_callback_t struct */
    const pb_arg_t *data = (const pb_arg_t *)(*arg);
    if (!data || !data->buffer) return true;
    if (!pb_encode_tag(os, PB_WT_STRING, field->tag)) return false;
    return pb_encode_string(os, data->buffer, data->length);
}

static bool decode_bytes_cb(void *stream, const pb_field_t *field, void **arg)
{
    pb_istream_t *is = (pb_istream_t *)stream;
    pb_arg_t *dest = (pb_arg_t *)*arg;
    (void)field;

    uint64_t len;
    if (!pb_decode_varint(is, &len)) return false;
    if (len > dest->length) return false;

    dest->length = (size_t)len;
    return pb_read(is, (uint8_t *)dest->buffer, dest->length);
}

/* ---------------------------------------------------------
 * Encrypt a protobuf payload into EncryptedMessage
 * --------------------------------------------------------- */
static size_t encrypt_message(const uint8_t *session_key,
                              uint32_t inner_type,
                              const uint8_t *plaintext,
                              size_t plaintext_len,
                              uint8_t *out)
{
    EncryptedMessage enc = EncryptedMessage_init_default;
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    uint8_t ciphertext[2048];
    unsigned long long clen;

    crypto_aead_chacha20poly1305_ietf_encrypt(ciphertext, &clen,
                                              plaintext, plaintext_len,
                                              NULL, 0, NULL, nonce, session_key);
    
    enc.type = inner_type; 
    
    pb_arg_t arg_payload = {ciphertext, (size_t)clen};
    enc.data.funcs.encode = encode_bytes_cb;
    enc.data.arg = &arg_payload;

    return encode_message(EncryptedMessage_fields, &enc, out);
}

/* ---------------------------------------------------------
 * Decrypt EncryptedMessage
 * --------------------------------------------------------- */
static bool decrypt_message(const uint8_t *session_key,
                              const uint8_t *payload,
                              size_t len,
                              uint8_t *out,
                              uint32_t *inner_type,
                              size_t *out_len)
{
    EncryptedMessage enc = EncryptedMessage_init_default;
    uint8_t ciphertext[2048];
    pb_arg_t arg = {ciphertext, sizeof(ciphertext)};
    
    enc.data.funcs.decode = decode_bytes_cb;
    enc.data.arg = &arg;

    pb_istream_t stream = pb_istream_from_buffer(payload, len);
    
    if (!pb_decode(&stream, EncryptedMessage_fields, &enc)) {
        printf("Failed to decode EncryptedMessage\n");
        return false;
    }
    
    if (inner_type) *inner_type = enc.type;

    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    unsigned long long plen;

    if (crypto_aead_chacha20poly1305_ietf_decrypt(out, &plen,
                                                  NULL,
                                                  ciphertext, arg.length,
                                                  NULL, 0, nonce, session_key) != 0)
    {
        printf("Decryption failed\n");
        return false;
    }
    
    if (out_len) *out_len = (size_t)plen;
    return true;
}

/* ---------------------------------------------------------
 * MAIN
 * --------------------------------------------------------- */
int main(int argc, char *argv[])
{
    const char *host = "127.0.0.1";
    int port         = 6053;
    const char *api_key = "123456789"; /* CHANGE */

    if (argc > 1) {
        host = argv[1];
    }
    if (argc > 2) {
        port = atoi(argv[2]);
    }
    if (argc > 3) {
        api_key = argv[3];
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "Failed to initialize libsodium\n");
        return 1;
    }

    printf("Connecting to %s:%d...\n", host, port);

    /* Connect TCP */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    if (connect(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }
    printf("Connected\n");

    /* ---------------- HELLO ---------------- */
    HelloRequest hello = HelloRequest_init_default;
    hello.api_version_major = 1;
    hello.api_version_minor = 9;

    // Set up callback to send client identification string
    pb_arg_t client_info_arg = {(const uint8_t *)"Esphome-c-client", 16};
    hello.client_info.funcs.encode = encode_bytes_cb;
    hello.client_info.arg = &client_info_arg;

    uint8_t buf[2048];
    size_t len = encode_message(HelloRequest_fields, &hello, buf);

    if (!send_frame(sock, MSG_HELLO_REQUEST, buf, len)) {
        fprintf(stderr, "Failed to send HelloRequest\n");
        return 1;
    }

    uint32_t type;
    uint8_t payload[2048];
    size_t plen;

    if (!recv_frame(sock, &type, payload, sizeof(payload), &plen)) {
        fprintf(stderr, "Failed to receive HelloResponse\n");
        return 1;
    }

    if (type != MSG_HELLO_RESPONSE) {
        fprintf(stderr, "Expected HelloResponse, got type %u\n", type);
        return 1;
    }
    printf("Received HelloResponse (type %u, len %zu)\n", type, plen);

    /* ---------------- ENCRYPTION REQUEST ---------------- */
    uint8_t client_nonce[32];
    /* Note: In a real scenario, this would be random and sent in EncryptionRequest.
     * Since the current proto is empty, we zero it out to match the fake device. */
    memset(client_nonce, 0, 32);

    EncryptionRequest ereq = EncryptionRequest_init_default;

    /* Note: EncryptionRequest in your current .pb.h is empty. 
     * You may need to update your .proto files if nonces are required. */

    len = encode_message(EncryptionRequest_fields, &ereq, buf);

    if (!send_frame(sock, MSG_ENCRYPTION_REQUEST, buf, len)) {
        fprintf(stderr, "Failed to send EncryptionRequest\n");
        return 1;
    }

    /* ---------------- ENCRYPTION RESPONSE ---------------- */
    if (!recv_frame(sock, &type, payload, sizeof(payload), &plen)) {
        fprintf(stderr, "Failed to receive EncryptionResponse\n");
        return 1;
    }

    EncryptionResponse eresp = EncryptionResponse_init_default;
    pb_istream_t istream_err = pb_istream_from_buffer(payload, plen);
    if (!pb_decode(&istream_err, EncryptionResponse_fields, &eresp)) {
        fprintf(stderr, "Failed to decode EncryptionResponse\n");
        return 1;
    }

    uint8_t server_nonce[32];
    memset(server_nonce, 0, 32); // Populate from eresp if fields existed

    /* ---------------- SESSION KEY ---------------- */
    uint8_t session_key[32];

    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, (const unsigned char*)api_key, strlen(api_key));
    crypto_auth_hmacsha256_update(&st, client_nonce, 32);
    crypto_auth_hmacsha256_update(&st, server_nonce, 32);
    crypto_auth_hmacsha256_final(&st, session_key);

    printf("Session key derived\n");

    /* ---------------- ENCRYPTED CONNECT REQUEST ---------------- */
    ConnectRequest creq = ConnectRequest_init_default;

    // Set up callback to send the actual API Key (password)
    pb_arg_t password_arg = {(const uint8_t *)api_key, strlen(api_key)};
    creq.password.funcs.encode = encode_bytes_cb;
    creq.password.arg = &password_arg;

    uint8_t plain[512];
    size_t plain_len = encode_message(
        ConnectRequest_fields,
        &creq, plain);

    size_t enc_len = encrypt_message(session_key, MSG_CONNECT_REQUEST, 
                                     plain, plain_len, buf);

    if (!send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, enc_len)) {
        fprintf(stderr, "Failed to send Encrypted ConnectRequest\n");
        return 1;
    }

    /* ---------------- ENCRYPTED CONNECT RESPONSE ---------------- */
    if (!recv_frame(sock, &type, payload, sizeof(payload), &plen)) {
        fprintf(stderr, "Failed to receive Encrypted ConnectResponse\n");
        return 1;
    }

    if (plen == 0) {
        fprintf(stderr, "Received empty response from server\n");
        return 1;
    }

    uint8_t decrypted[512];
    uint32_t inner_type;
    size_t dlen;
    if (!decrypt_message(session_key, payload, plen, decrypted, &inner_type, &dlen)) return 1;

    ConnectResponse cres = ConnectResponse_init_default;
    pb_istream_t istream_conn = pb_istream_from_buffer(decrypted, dlen);
    if (!pb_decode(&istream_conn, ConnectResponse_fields, &cres)) {
        fprintf(stderr, "Failed to decode ConnectResponse\n");
        return 1;
    }

    printf("Connected (encrypted). invalid_password=%d\n", cres.invalid_password);

    if (cres.invalid_password) {
        close(sock);
        return 1;
    }

    /* ---------------- LIST ENTITIES ---------------- */
    printf("Requesting entities...\n");
    ListEntitiesRequest lreq = {0}; // Empty message
    size_t lreq_len = encode_message(ListEntitiesRequest_fields, &lreq, plain);
    size_t lreq_enc_len = encrypt_message(session_key, MSG_LIST_ENTITIES_REQUEST,
                                          plain, lreq_len, buf);
    send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, lreq_enc_len);

    bool list_done = false;
    while (!list_done) {
        if (!recv_frame(sock, &type, payload, sizeof(payload), &plen)) break;
        
        if (!decrypt_message(session_key, payload, plen, decrypted, &inner_type, &dlen)) continue;

        pb_istream_t is = pb_istream_from_buffer(decrypted, dlen);
        char name_buf[128];
        pb_arg_t name_arg = {(uint8_t*)name_buf, sizeof(name_buf) - 1};

        switch (inner_type) {
            case MSG_LIST_ENTITIES_SENSOR_RESPONSE: {
                ListEntitiesSensorResponse res = ListEntitiesSensorResponse_init_default;
                res.name.funcs.decode = decode_bytes_cb;
                res.name.arg = &name_arg;
                if (pb_decode(&is, ListEntitiesSensorResponse_fields, &res)) {
                    name_buf[name_arg.length] = '\0';
                    printf(" [Sensor] Name: %s, Key: %u\n", name_buf, res.key);
                }
                break;
            }
            case MSG_LIST_ENTITIES_BINARY_SENSOR_RESPONSE: {
                ListEntitiesBinarySensorResponse res = ListEntitiesBinarySensorResponse_init_default;
                res.name.funcs.decode = decode_bytes_cb;
                res.name.arg = &name_arg;
                if (pb_decode(&is, ListEntitiesBinarySensorResponse_fields, &res)) {
                    name_buf[name_arg.length] = '\0';
                    printf(" [Binary Sensor] Name: %s, Key: %u\n", name_buf, res.key);
                }
                break;
            }
            case MSG_LIST_ENTITIES_SWITCH_RESPONSE: {
                ListEntitiesSwitchResponse res = ListEntitiesSwitchResponse_init_default;
                res.name.funcs.decode = decode_bytes_cb;
                res.name.arg = &name_arg;
                if (pb_decode(&is, ListEntitiesSwitchResponse_fields, &res)) {
                    name_buf[name_arg.length] = '\0';
                    printf(" [Switch] Name: %s, Key: %u\n", name_buf, res.key);
                }
                break;
            }
            case MSG_LIST_ENTITIES_LIGHT_RESPONSE: {
                ListEntitiesLightResponse res = ListEntitiesLightResponse_init_default;
                res.name.funcs.decode = decode_bytes_cb;
                res.name.arg = &name_arg;
                if (pb_decode(&is, ListEntitiesLightResponse_fields, &res)) {
                    name_buf[name_arg.length] = '\0';
                    printf(" [Light] Name: %s, Key: %u\n", name_buf, res.key);
                }
                break;
            }
            case MSG_LIST_ENTITIES_DONE_RESPONSE:
                printf("Entity listing complete.\n");
                list_done = true;
                break;
            default:
                printf("Received other encrypted message type: %u\n", inner_type);
        }
    }

    /* ---------------- SUBSCRIBE STATES ---------------- */
    printf("Subscribing to state updates...\n");
    SubscribeStatesRequest sreq = SubscribeStatesRequest_init_default;
    size_t sreq_len = encode_message(SubscribeStatesRequest_fields, &sreq, plain);
    size_t sreq_enc_len = encrypt_message(session_key, MSG_SUBSCRIBE_STATES_REQUEST,
                                          plain, sreq_len, buf);
    send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, sreq_enc_len);

    /* ---------------- SWITCH COMMAND ---------------- */
    // Example: Toggle the "Fake Switch" (Key: 3) to ON
    printf("Sending SwitchCommandRequest (Key: 3, State: ON)...\n");
    SwitchCommandRequest swreq = SwitchCommandRequest_init_default;
    swreq.key = 3;
    swreq.state = true;
    size_t swreq_len = encode_message(SwitchCommandRequest_fields, &swreq, plain);
    size_t swreq_enc_len = encrypt_message(session_key, MSG_SWITCH_COMMAND_REQUEST,
                                          plain, swreq_len, buf);
    send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, swreq_enc_len);

    /* ---------------- EVENT LOOP ---------------- */
    printf("Listening for state updates (Ctrl+C to stop)...\n");
    time_t last_ping = time(NULL);
    int ping_count = 0;
    bool keep_running = true;
    bool is_fake_device = (strcmp(host, "127.0.0.1") == 0);

    while (keep_running) {
        fd_set rfds;
        struct timeval tv;
        int retval;

        FD_ZERO(&rfds);
        FD_SET(sock, &rfds);

        tv.tv_sec = 1; // Check timing every second
        tv.tv_usec = 0;

        retval = select(sock + 1, &rfds, NULL, NULL, &tv);

        if (retval == -1) {
            perror("select()");
            break;
        }

        time_t now = time(NULL);
        if (now - last_ping >= 10 && (!is_fake_device || ping_count < 5)) {
            PingRequest preq = PingRequest_init_default;
            size_t preq_len = encode_message(PingRequest_fields, &preq, plain);
            size_t preq_enc_len = encrypt_message(session_key, MSG_PING_REQUEST,
                                                  plain, preq_len, buf);
            if (send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, preq_enc_len)) {
                printf("Sent Ping\n");
                last_ping = now;
                ping_count++;
            }
        }

        if (retval > 0 && FD_ISSET(sock, &rfds)) {
            if (!recv_frame(sock, &type, payload, sizeof(payload), &plen)) {
                printf("Connection closed by device.\n");
                break;
            }
            
            if (!decrypt_message(session_key, payload, plen, decrypted, &inner_type, &dlen)) continue;

            pb_istream_t is = pb_istream_from_buffer(decrypted, dlen);

            switch (inner_type) {
            case MSG_SENSOR_STATE_RESPONSE: {
                SensorStateResponse res = SensorStateResponse_init_default;
                if (pb_decode(&is, SensorStateResponse_fields, &res)) {
                    printf(" [UPDATE] Sensor Key: %u, State: %.2f\n", res.key, res.state);
                }
                break;
            }
            case MSG_BINARY_SENSOR_STATE_RESPONSE: {
                BinarySensorStateResponse res = BinarySensorStateResponse_init_default;
                if (pb_decode(&is, BinarySensorStateResponse_fields, &res)) {
                    printf(" [UPDATE] Binary Sensor Key: %u, State: %s\n", 
                           res.key, res.state ? "ON" : "OFF");
                }
                break;
            }
            case MSG_SWITCH_STATE_RESPONSE: {
                SwitchStateResponse res = SwitchStateResponse_init_default;
                if (pb_decode(&is, SwitchStateResponse_fields, &res)) {
                    printf(" [UPDATE] Switch Key: %u, State: %s\n", 
                           res.key, res.state ? "ON" : "OFF");
                }
                break;
            }
            case MSG_PING_RESPONSE:
                if (is_fake_device) {
                    printf("Received Pong (%d/5)\n", ping_count);
                    if (ping_count >= 5) {
                        printf("Completed 5 ping cycles with fake_device. Exiting loop.\n");
                        keep_running = false;
                    }
                } else {
                    printf("Received Pong\n");
                }
                break;
            default:
                printf("Received unsolicited encrypted message type: %u\n", inner_type);
            }
        }
    }

    close(sock);
    return 0;
}