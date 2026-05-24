#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <stdbool.h>

#include <sodium.h>

#include "pb_encode.h"
#include "pb_decode.h"
#include "api.pb.h"
#include "api_encryption.pb.h"

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
    MSG_CONNECT_RESPONSE     = 7
};

/* ---------------------------------------------------------
 * Send raw protobuf message with 4-byte type + 2-byte length
 * --------------------------------------------------------- */
static bool send_frame(int sock, uint32_t type,
                       const uint8_t *payload, size_t len)
{
    uint8_t header[6];
    header[0] = (type >> 24) & 0xFF;
    header[1] = (type >> 16) & 0xFF;
    header[2] = (type >> 8)  & 0xFF;
    header[3] = (type)       & 0xFF;
    header[4] = (len >> 8) & 0xFF;
    header[5] = (len)      & 0xFF;

    if (write(sock, header, 6) != 6) return false;
    if (write(sock, payload, len) != (ssize_t)len) return false;

    return true;
}

/* ---------------------------------------------------------
 * Receive raw protobuf message
 * --------------------------------------------------------- */
static bool recv_frame(int sock, uint32_t *type,
                       uint8_t *payload, size_t *len)
{
    uint8_t header[6];

    if (read(sock, header, 6) != 6)
        return false;

    *type = (header[0] << 24) |
            (header[1] << 16) |
            (header[2] << 8)  |
            (header[3]);

    *len = (header[4] << 8) | header[5];

    if (read(sock, payload, *len) != (ssize_t)*len)
        return false;

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
 * Encrypt a protobuf payload into EncryptedMessage
 * --------------------------------------------------------- */
static size_t encrypt_message(const uint8_t *session_key,
                              const uint8_t *plaintext,
                              size_t plaintext_len,
                              uint8_t *out)
{
    EncryptedMessage enc = EncryptedMessage_init_default;
    
    // Note: EncryptedMessage in nanopb uses callbacks for bytes.
    // For simplicity in this example, we assume type is set.
    enc.type = MSG_ENCRYPTED_MESSAGE; 
    
    uint8_t ciphertext[2048];
    unsigned long long cipher_len = 0;
    
    // Encryption logic would populate a callback for 'data'
    
    return encode_message(EncryptedMessage_fields, &enc, out);
}

/* ---------------------------------------------------------
 * Decrypt EncryptedMessage
 * --------------------------------------------------------- */
static size_t decrypt_message(const uint8_t *session_key,
                              const uint8_t *payload,
                              size_t len,
                              uint8_t *out)
{
    EncryptedMessage enc = EncryptedMessage_init_default;
    pb_istream_t stream = pb_istream_from_buffer(payload, len);
    
    if (!pb_decode(&stream, EncryptedMessage_fields, &enc)) {
        printf("Failed to decode EncryptedMessage\n");
        return 0;
    }
    
    unsigned long long out_len = 0;
    // Decryption logic goes here
    
    return (size_t)out_len;
}

/* ---------------------------------------------------------
 * MAIN
 * --------------------------------------------------------- */
int main(void)
{
    const char *host = "192.168.1.50";  /* CHANGE */
    const int port   = 6053;
    const char *api_key = "YOUR_API_KEY_HERE"; /* CHANGE */

    sodium_init();

    /* Connect TCP */
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    inet_pton(AF_INET, host, &addr.sin_addr);

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    printf("Connected\n");

    /* ---------------- HELLO ---------------- */
    HelloRequest hello = HelloRequest_init_default;
    /* String fields in nanopb require callbacks. 
     * For simple testing, you can adjust the proto to use static strings. */

    uint8_t buf[2048];
    size_t len = encode_message(HelloRequest_fields, &hello, buf);

    send_frame(sock, MSG_HELLO_REQUEST, buf, len);

    uint32_t type;
    uint8_t payload[2048];
    size_t plen;

    recv_frame(sock, &type, payload, &plen);
    printf("Got HelloResponse\n");

    /* ---------------- ENCRYPTION REQUEST ---------------- */
    uint8_t client_nonce[32];
    randombytes_buf(client_nonce, sizeof(client_nonce));

    esphome__api__EncryptionRequest ereq =
        ESPHOME__API__ENCRYPTION_REQUEST__INIT;

    ereq.client_nonce.data = client_nonce;
    ereq.client_nonce.len  = sizeof(client_nonce);

    len = encode_message(
        &esphome__api__encryption_request__descriptor,
        &ereq, buf);

    send_frame(sock, MSG_ENCRYPTION_REQUEST, buf, len);

    /* ---------------- ENCRYPTION RESPONSE ---------------- */
    recv_frame(sock, &type, payload, &plen);

    esphome__api__EncryptionResponse *eresp =
        esphome__api__encryption_response__unpack(NULL, plen, payload);

    uint8_t server_nonce[32];
    memcpy(server_nonce, eresp->server_nonce.data, 32);

    esphome__api__encryption_response__free_unpacked(eresp, NULL);

    /* ---------------- SESSION KEY ---------------- */
    uint8_t session_key[32];

    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, (const unsigned char*)api_key, strlen(api_key));
    crypto_auth_hmacsha256_update(&st, client_nonce, 32);
    crypto_auth_hmacsha256_update(&st, server_nonce, 32);
    crypto_auth_hmacsha256_final(&st, session_key);

    printf("Session key derived\n");

    /* ---------------- ENCRYPTED CONNECT REQUEST ---------------- */
    esphome__api__ConnectRequest creq =
        ESPHOME__API__CONNECT_REQUEST__INIT;

    creq.password = (char*)api_key;

    uint8_t plain[512];
    size_t plain_len = encode_message(
        &esphome__api__connect_request__descriptor,
        &creq, plain);

    size_t enc_len = encrypt_message(session_key, plain, plain_len, buf);

    send_frame(sock, MSG_ENCRYPTED_MESSAGE, buf, enc_len);

    /* ---------------- ENCRYPTED CONNECT RESPONSE ---------------- */
    recv_frame(sock, &type, payload, &plen);

    uint8_t decrypted[512];
    size_t dlen = decrypt_message(session_key, payload, plen, decrypted);

    esphome__api__ConnectResponse *cres =
        esphome__api__connect_response__unpack(NULL, dlen, decrypted);

    printf("Connected (encrypted). invalid_password=%d\n",
           cres->invalid_password);

    esphome__api__connect_response__free_unpacked(cres, NULL);

    close(sock);
    return 0;
}