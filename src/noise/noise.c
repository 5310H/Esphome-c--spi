#include "esphome_noise.h"
#include <string.h>

// Placeholder Noise-PSK implementation scaffolding.
// Real crypto will be added later.

int esph_noise_init(esph_noise_ctx_t *ctx, const char *psk) {
    memset(ctx, 0, sizeof(*ctx));
    (void)psk; // unused for now
    return 0;
}

int esph_noise_handshake(esph_noise_ctx_t *ctx, int sock) {
    (void)ctx;
    (void)sock;
    // TODO: implement Noise-PSK handshake
    return 0;
}

int esph_noise_encrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len) {
    (void)ctx;
    // TODO: replace with real encryption
    memcpy(out, in, in_len);
    *out_len = in_len;
    return 0;
}

int esph_noise_decrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len) {
    (void)ctx;
    // TODO: replace with real decryption
    memcpy(out, in, in_len);
    *out_len = in_len;
    return 0;
}
