/* FULL FAKE ESPHOME DEVICE WITH ENCRYPTED ENTITIES */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <sodium.h>

#include "proto/api_message.pb.h"
#include "proto/api_service.pb.h"
#include "proto/api_encryption.pb.h"
#include "proto/api_options.pb.h"

#include "proto/pb_encode.h"
#include "proto/pb_decode.h"

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
   Nanopb byte callbacks
--------------------------------------------------------- */
typedef struct {
    const uint8_t *buffer;
    size_t length;
} pb_arg_t;

static bool encode_bytes_cb(void *stream, const pb_field_t *field, void * const *arg) {
    pb_ostream_t *os = (pb_ostream_t *)stream;
    const pb_arg_t *data = (const pb_arg_t *)*arg;
    if (!data || !data->buffer) return true;
    if (!pb_encode_tag(os, PB_WT_STRING, field->tag)) return false;
    return pb_encode_string(os, data->buffer, data->length);
}

static bool decode_bytes_cb(void *stream, const pb_field_t *field, void **arg) {
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
   4-byte type + 2-byte length framing
--------------------------------------------------------- */
static bool send_frame(int fd, uint32_t type, const uint8_t *data, size_t len) {
    uint8_t header[6];
    header[0] = (type >> 24) & 0xFF;
    header[1] = (type >> 16) & 0xFF;
    header[2] = (type >> 8) & 0xFF;
    header[3] = (type) & 0xFF;
    header[4] = (len >> 8) & 0xFF;
    header[5] = (len) & 0xFF;

    size_t total_sent = 0;
    while (total_sent < 6) {
        ssize_t n = send(fd, header + total_sent, 6 - total_sent, MSG_NOSIGNAL);
        if (n <= 0) return false;
        total_sent += n;
    }

    total_sent = 0;
    while (total_sent < len) {
        ssize_t n = send(fd, data + total_sent, len - total_sent, MSG_NOSIGNAL);
        if (n <= 0) return false;
        total_sent += n;
    }

    printf("FAKE DEVICE: Sent frame type=%u len=%zu\n", type, len);
    return true;
}

static bool recv_frame(int fd, uint32_t *type, uint8_t *payload, size_t max_len, size_t *len) {
    uint8_t header[6];
    size_t total_read = 0;
    while (total_read < 6) {
        ssize_t n = recv(fd, header + total_read, 6 - total_read, 0);
        if (n <= 0) return false;
        total_read += n;
    }

    *type = (header[0] << 24) | (header[1] << 16) | (header[2] << 8) | header[3];
    *len  = (header[4] << 8) | header[5];

    if (*len > max_len) return false;

    total_read = 0;
    while (total_read < *len) {
        ssize_t n = recv(fd, payload + total_read, *len - total_read, 0);
        if (n <= 0) return false;
        total_read += n;
    }

    printf("FAKE DEVICE: Received frame type=%u len=%zu\n", *type, *len);
    return true;
}

/* ---------------------------------------------------------
   Encrypt helper
--------------------------------------------------------- */
static size_t encrypt_message(
    const uint8_t *session_key,
    uint32_t inner_type,
    const uint8_t *plaintext,
    size_t plaintext_len,
    uint8_t *out)
{
    EncryptedMessage enc = EncryptedMessage_init_default;

    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};
    uint8_t ciphertext[2048];
    unsigned long long clen;

    crypto_aead_chacha20poly1305_ietf_encrypt(
        ciphertext, &clen,
        plaintext, plaintext_len,
        NULL, 0, NULL,
        nonce, session_key
    );

    enc.type = inner_type;

    pb_arg_t arg_payload = {ciphertext, (size_t)clen};
    enc.data.funcs.encode = encode_bytes_cb;
    enc.data.arg = &arg_payload;

    pb_ostream_t os = pb_ostream_from_buffer(out, 2048);
    pb_encode(&os, EncryptedMessage_fields, &enc);
    return os.bytes_written;
}

/* ---------------------------------------------------------
   Decrypt helper
--------------------------------------------------------- */
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
   MAIN
--------------------------------------------------------- */
int main() {
    if (sodium_init() < 0) return 1;

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6053);

    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 1);

    printf("FAKE DEVICE: Waiting on port 6053\n");

    socklen_t addrlen = sizeof(addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    printf("FAKE DEVICE: Client connected\n");

    const char *api_key = "123456789";
    uint8_t session_key[32];
    uint8_t client_nonce[32] = {0};
    uint8_t server_nonce[32] = {0};

    uint32_t type_in;
    size_t len_in;
    uint8_t recvbuf[2048];
    uint8_t sendbuf[2048];

    /* ---------------------------------------------------------
       1. HelloRequest -> HelloResponse
    --------------------------------------------------------- */
    recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in);

    HelloResponse hello = HelloResponse_init_default;
    hello.api_version_major = 1;
    hello.api_version_minor = 9;

    pb_ostream_t os = pb_ostream_from_buffer(sendbuf, sizeof(sendbuf));
    pb_encode(&os, HelloResponse_fields, &hello);
    send_frame(client_fd, 2, sendbuf, os.bytes_written);

    /* ---------------------------------------------------------
       2. EncryptionRequest -> EncryptionResponse
    --------------------------------------------------------- */
    recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in);

    EncryptionResponse eresp = EncryptionResponse_init_default;
    os = pb_ostream_from_buffer(sendbuf, sizeof(sendbuf));
    pb_encode(&os, EncryptionResponse_fields, &eresp);
    send_frame(client_fd, 4, sendbuf, os.bytes_written);

    /* ---------------------------------------------------------
       3. Derive session key
    --------------------------------------------------------- */
    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, (const unsigned char*)api_key, strlen(api_key));
    crypto_auth_hmacsha256_update(&st, client_nonce, 32);
    crypto_auth_hmacsha256_update(&st, server_nonce, 32);
    crypto_auth_hmacsha256_final(&st, session_key);

    /* ---------------------------------------------------------
       4. Receive encrypted ConnectRequest
    --------------------------------------------------------- */
    recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in);

    EncryptedMessage enc_in = EncryptedMessage_init_default;
    uint8_t ciphertext[2048];
    pb_arg_t dec_arg = {ciphertext, sizeof(ciphertext)};
    enc_in.data.funcs.decode = decode_bytes_cb;
    enc_in.data.arg = &dec_arg;

    pb_istream_t is = pb_istream_from_buffer(recvbuf, len_in);
    pb_decode(&is, EncryptedMessage_fields, &enc_in);

    uint8_t decrypted[512];
    unsigned long long plen;
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};

    if (crypto_aead_chacha20poly1305_ietf_decrypt(
        decrypted, &plen, NULL,
        ciphertext, dec_arg.length,
        NULL, 0, nonce, session_key
    ) != 0) {
        printf("FAKE DEVICE: ConnectRequest decryption failed\n");
        return 1;
    }

    printf("FAKE DEVICE: Decrypted ConnectRequest OK\n");

    /* ---------------------------------------------------------
       5. Send encrypted ConnectResponse
    --------------------------------------------------------- */
    ConnectResponse cres = ConnectResponse_init_default;
    cres.invalid_password = false;

    uint8_t plain_resp[256];
    os = pb_ostream_from_buffer(plain_resp, sizeof(plain_resp));
    pb_encode(&os, ConnectResponse_fields, &cres);

    size_t enc_len = encrypt_message(
        session_key, 7, plain_resp, os.bytes_written, sendbuf
    );

    send_frame(client_fd, 5, sendbuf, enc_len);

    /* ---------------------------------------------------------
       6. Receive encrypted ListEntitiesRequest
    --------------------------------------------------------- */
    recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in);

    EncryptedMessage enc_list = EncryptedMessage_init_default;
    uint8_t list_cipher[2048];
    pb_arg_t list_dec_arg = {list_cipher, sizeof(list_cipher)};
    enc_list.data.funcs.decode = decode_bytes_cb;
    enc_list.data.arg = &list_dec_arg;

    is = pb_istream_from_buffer(recvbuf, len_in);
    pb_decode(&is, EncryptedMessage_fields, &enc_list);

    uint8_t list_plain[256];
    unsigned long long list_plen;

    if (crypto_aead_chacha20poly1305_ietf_decrypt(
        list_plain, &list_plen, NULL,
        list_cipher, list_dec_arg.length,
        NULL, 0, nonce, session_key
    ) != 0) {
        printf("FAKE DEVICE: ListEntitiesRequest decryption failed\n");
        return 1;
    }

    printf("FAKE DEVICE: Got ListEntitiesRequest\n");

    /* ---------------------------------------------------------
       7. Send fake entities (Sensor + Binary + Switch + Light)
    --------------------------------------------------------- */
    const char *names[] = {
        "Fake Sensor",
        "Fake Binary Sensor",
        "Fake Switch",
        "Fake Light"
    };

    uint32_t types[] = {
        12, // Sensor
        13, // Binary Sensor
        14, // Switch
        15  // Light
    };

    for (int i = 0; i < 4; i++) {
        uint8_t plain_ent[256];
        os = pb_ostream_from_buffer(plain_ent, sizeof(plain_ent));

        pb_arg_t name_arg = {(const uint8_t*)names[i], strlen(names[i])};

        switch (types[i]) {
            case 12: {
                ListEntitiesSensorResponse s = ListEntitiesSensorResponse_init_default;
                s.key = i + 1;
                s.name.funcs.encode = encode_bytes_cb;
                s.name.arg = &name_arg;
                pb_encode(&os, ListEntitiesSensorResponse_fields, &s);
                break;
            }
            case 13: {
                ListEntitiesBinarySensorResponse s = ListEntitiesBinarySensorResponse_init_default;
                s.key = i + 1;
                s.name.funcs.encode = encode_bytes_cb;
                s.name.arg = &name_arg;
                pb_encode(&os, ListEntitiesBinarySensorResponse_fields, &s);
                break;
            }
            case 14: {
                ListEntitiesSwitchResponse s = ListEntitiesSwitchResponse_init_default;
                s.key = i + 1;
                s.name.funcs.encode = encode_bytes_cb;
                s.name.arg = &name_arg;
                pb_encode(&os, ListEntitiesSwitchResponse_fields, &s);
                break;
            }
            case 15: {
                ListEntitiesLightResponse s = ListEntitiesLightResponse_init_default;
                s.key = i + 1;
                s.name.funcs.encode = encode_bytes_cb;
                s.name.arg = &name_arg;
                pb_encode(&os, ListEntitiesLightResponse_fields, &s);
                break;
            }
        }

        size_t ent_len = encrypt_message(
            session_key, types[i], plain_ent, os.bytes_written, sendbuf
        );

        send_frame(client_fd, 5, sendbuf, ent_len);
    }

    /* ---------------------------------------------------------
       8. DONE
    --------------------------------------------------------- */
    ListEntitiesDoneResponse done = ListEntitiesDoneResponse_init_default;

    uint8_t plain_done[64];
    os = pb_ostream_from_buffer(plain_done, sizeof(plain_done));
    pb_encode(&os, ListEntitiesDoneResponse_fields, &done);

    size_t done_len = encrypt_message(
        session_key, 19, plain_done, os.bytes_written, sendbuf
    );

    send_frame(client_fd, 5, sendbuf, done_len);

    printf("FAKE DEVICE: Sent all entities + DONE\n");

    /* ---------------------------------------------------------
       9. Event Loop (Simulator)
    --------------------------------------------------------- */
    printf("FAKE DEVICE: Session active. Handling requests...\n");
    while (recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in)) {
        uint32_t inner_type_in;
        uint8_t inner_plain[512];
        size_t inner_plen;
        
        if (!decrypt_message(session_key, recvbuf, len_in, inner_plain, &inner_type_in, &inner_plen)) {
            continue;
        }
        
        printf("FAKE DEVICE: Received decrypted inner type %u\n", inner_type_in);
        
        if (inner_type_in == MSG_SUBSCRIBE_STATES_REQUEST) {
            printf("FAKE DEVICE: Client subscribed to states. Sending mock update...\n");
            SensorStateResponse state = SensorStateResponse_init_default;
            state.key = 1;
            state.state = 23.5f;
            uint8_t state_plain[64];
            os = pb_ostream_from_buffer(state_plain, sizeof(state_plain));
            pb_encode(&os, SensorStateResponse_fields, &state);
            size_t state_enc_len = encrypt_message(session_key, MSG_SENSOR_STATE_RESPONSE, state_plain, os.bytes_written, sendbuf);
            send_frame(client_fd, MSG_ENCRYPTED_MESSAGE, sendbuf, state_enc_len);
        } else if (inner_type_in == MSG_PING_REQUEST) {
            printf("FAKE DEVICE: Received Ping. Sending Pong...\n");
            PingResponse pres = PingResponse_init_default;
            uint8_t plain_pong[64];
            os = pb_ostream_from_buffer(plain_pong, sizeof(plain_pong));
            pb_encode(&os, PingResponse_fields, &pres);
            size_t pong_enc_len = encrypt_message(session_key, MSG_PING_RESPONSE, plain_pong, os.bytes_written, sendbuf);
            send_frame(client_fd, MSG_ENCRYPTED_MESSAGE, sendbuf, pong_enc_len);
        } else if (inner_type_in == MSG_SWITCH_COMMAND_REQUEST) {
            SwitchCommandRequest sw_in = SwitchCommandRequest_init_default;
            pb_istream_t is_sw = pb_istream_from_buffer(inner_plain, inner_plen);
            if (pb_decode(&is_sw, SwitchCommandRequest_fields, &sw_in)) {
                printf("FAKE DEVICE: Received SwitchCommand for Key %u -> %s\n", 
                       sw_in.key, sw_in.state ? "ON" : "OFF");
                
                // Respond with state update confirmation
                SwitchStateResponse sw_out = SwitchStateResponse_init_default;
                sw_out.key = sw_in.key;
                sw_out.state = sw_in.state;
                uint8_t sw_plain[64];
                os = pb_ostream_from_buffer(sw_plain, sizeof(sw_plain));
                pb_encode(&os, SwitchStateResponse_fields, &sw_out);
                size_t sw_enc_len = encrypt_message(session_key, MSG_SWITCH_STATE_RESPONSE, sw_plain, os.bytes_written, sendbuf);
                send_frame(client_fd, MSG_ENCRYPTED_MESSAGE, sendbuf, sw_enc_len);
            }
        }
    }

    printf("FAKE DEVICE: Connection closed.\n");

    close(client_fd);
    close(server_fd);
    return 0;
}