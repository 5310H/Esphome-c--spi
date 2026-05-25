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

/* ---------------------------------------------------------
 * Nanopb callbacks for encoding/decoding bytes
 * --------------------------------------------------------- */
typedef struct {
    const uint8_t *buffer;
    size_t length;
} pb_arg_t;

static bool encode_bytes_cb(void *stream, const pb_field_t *field, void * const *arg)
{
    pb_ostream_t *os = (pb_ostream_t *)stream;
    const pb_arg_t *data = (const pb_arg_t *)*arg;
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
   Send framed ESPHome message
   --------------------------------------------------------- */
static bool send_frame(int fd, uint32_t type, const uint8_t *data, size_t len) {
    uint8_t header[6];
    header[0] = (type >> 24) & 0xFF;
    header[1] = (type >> 16) & 0xFF;
    header[2] = (type >> 8)  & 0xFF;
    header[3] = (type)       & 0xFF;
    header[4] = (len >> 8) & 0xFF;
    header[5] = (len)      & 0xFF;

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

    printf("FAKE DEVICE: Sent frame type=%u len=%zu\n", (unsigned int)type, len);
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
    *len = (header[4] << 8) | header[5];

    if (*len > max_len) {
        printf("FAKE DEVICE: Received frame too large (%zu > %zu)\n", *len, max_len);
        return false;
    }

    total_read = 0;
    while (total_read < *len) {
        ssize_t n = recv(fd, payload + total_read, *len - total_read, 0);
        if (n <= 0) return false;
        total_read += n;
    }

    printf("FAKE DEVICE: Received frame type=%u len=%zu\n", (unsigned int)*type, *len);
    return true;
}

/* ---------------------------------------------------------
   MAIN
   --------------------------------------------------------- */
int main() {
    if (sodium_init() < 0) return 1;

    int server_fd, client_fd;
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(6053);

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        return 1;
    }
    listen(server_fd, 1);

    printf("FAKE DEVICE: Waiting on port 6053\n");
    client_fd = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        return 1;
    }
    printf("FAKE DEVICE: Client connected\n");

    const char *api_key = "123456789";
    uint32_t type_in;
    size_t len_in;
    uint8_t recvbuf[1024];
    uint8_t sendbuf[1024];
    pb_ostream_t ostream;

    /* ---------------------------------------------------------
       1. Receive HelloRequest -> Send HelloResponse
       --------------------------------------------------------- */
    if (!recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in)) {
        printf("FAKE DEVICE: Failed to receive HelloRequest\n");
        return 1;
    }

    HelloResponse hello = HelloResponse_init_default;
    hello.api_version_major = 1;
    hello.api_version_minor = 9;

    ostream = pb_ostream_from_buffer(sendbuf, sizeof(sendbuf));
    if (!pb_encode(&ostream, HelloResponse_fields, &hello)) {
        fprintf(stderr, "FAKE DEVICE: HelloResponse encode failed\n");
        return 1;
    }
    if (!send_frame(client_fd, 2, sendbuf, ostream.bytes_written)) {
        return 1;
    }
    printf("FAKE DEVICE: Sent HelloResponse\n");

    /* ---------------------------------------------------------
       2. Receive EncryptionRequest -> Send EncryptionResponse
       --------------------------------------------------------- */
    if (!recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in)) {
        printf("FAKE DEVICE: Failed to receive EncryptionRequest\n");
        return 1;
    }

    EncryptionResponse eresp = EncryptionResponse_init_default;
    ostream = pb_ostream_from_buffer(sendbuf, sizeof(sendbuf));
    if (!pb_encode(&ostream, EncryptionResponse_fields, &eresp)) {
        fprintf(stderr, "FAKE DEVICE: EncryptionResponse encode failed\n");
        return 1;
    }
    if (!send_frame(client_fd, 4, sendbuf, ostream.bytes_written)) {
        return 1;
    }
    printf("FAKE DEVICE: Sent EncryptionResponse\n");

    /* ---------------------------------------------------------
       3. Key Derivation (Match Client)
       --------------------------------------------------------- */
    uint8_t session_key[32];
    uint8_t client_nonce[32] = {0}; 
    uint8_t server_nonce[32] = {0};

    crypto_auth_hmacsha256_state st;
    crypto_auth_hmacsha256_init(&st, (const unsigned char*)api_key, strlen(api_key));
    crypto_auth_hmacsha256_update(&st, client_nonce, 32);
    crypto_auth_hmacsha256_update(&st, server_nonce, 32);
    crypto_auth_hmacsha256_final(&st, session_key);

    /* ---------------------------------------------------------
       4. Receive Encrypted ConnectRequest
       --------------------------------------------------------- */
    if (!recv_frame(client_fd, &type_in, recvbuf, sizeof(recvbuf), &len_in)) {
        printf("FAKE DEVICE: Failed to receive Encrypted ConnectRequest\n");
        return 1;
    }

    if (type_in != 5) { // MSG_ENCRYPTED_MESSAGE
        fprintf(stderr, "FAKE DEVICE: Expected type 5, got %u\n", type_in);
        return 1;
    }

    EncryptedMessage enc_in = EncryptedMessage_init_default;
    uint8_t ciphertext[1024];
    pb_arg_t decrypt_arg = {ciphertext, sizeof(ciphertext)};
    enc_in.data.funcs.decode = decode_bytes_cb;
    enc_in.data.arg = &decrypt_arg;

    pb_istream_t istream = pb_istream_from_buffer(recvbuf, len_in);
    if (!pb_decode(&istream, EncryptedMessage_fields, &enc_in)) {
        printf("FAKE DEVICE: Failed to decode EncryptedMessage\n");
        return 1;
    }

    uint8_t decrypted[512];
    unsigned long long plen;
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES] = {0};

    if (crypto_aead_chacha20poly1305_ietf_decrypt(decrypted, &plen, NULL,
            ciphertext, decrypt_arg.length, NULL, 0, nonce, session_key) != 0) {
        printf("FAKE DEVICE: Decryption failed\n");
    } else {
        printf("FAKE DEVICE: Decrypted ConnectRequest successfully\n");
    }

    /* ---------------------------------------------------------
       5. Encrypt and Send ConnectResponse
       --------------------------------------------------------- */
    ConnectResponse cres = ConnectResponse_init_default;
    cres.invalid_password = false;
    uint8_t plain_resp[128];
    ostream = pb_ostream_from_buffer(plain_resp, sizeof(plain_resp));
    pb_encode(&ostream, ConnectResponse_fields, &cres);

    uint8_t resp_cipher[256];
    unsigned long long clen;
    crypto_aead_chacha20poly1305_ietf_encrypt(resp_cipher, &clen,
        plain_resp, ostream.bytes_written, NULL, 0, NULL, nonce, session_key);

    EncryptedMessage enc_resp = EncryptedMessage_init_default;
    enc_resp.type = 7; // MSG_CONNECT_RESPONSE
    pb_arg_t encrypt_arg = {resp_cipher, (size_t)clen};
    enc_resp.data.funcs.encode = encode_bytes_cb;
    enc_resp.data.arg = &encrypt_arg;

    ostream = pb_ostream_from_buffer(sendbuf, sizeof(sendbuf));
    if (!pb_encode(&ostream, EncryptedMessage_fields, &enc_resp)) return 1;
    if (!send_frame(client_fd, 5, sendbuf, ostream.bytes_written)) return 1;
    
    printf("FAKE DEVICE: Done\n");

    close(client_fd);
    close(server_fd);
    return 0;
}