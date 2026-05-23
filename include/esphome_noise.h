
#ifndef ESPHOME_NOISE_H
#define ESPHOME_NOISE_H

#include <stdint.h>
#include <stddef.h>

// Noise-PSK handshake + encryption context
typedef struct {
    uint8_t handshake_hash[32];
    uint8_t send_key[32];
    uint8_t recv_key[32];
    uint64_t send_nonce;
    uint64_t recv_nonce;
} esph_noise_ctx_t;

// Initialize Noise context with PSK
int esph_noise_init(esph_noise_ctx_t *ctx, const char *psk);

// Perform full Noise handshake over a TCP socket
int esph_noise_handshake(esph_noise_ctx_t *ctx, int sock);

// Encrypt a plaintext frame
int esph_noise_encrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len);

// Decrypt a ciphertext frame
int esph_noise_decrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len);

#endif
