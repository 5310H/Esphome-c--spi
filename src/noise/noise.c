#include "esphome_noise.h"
#include "esphome_transport.h"

#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ecdh.h>
#include <mbedtls/chachapoly.h>
#include <mbedtls/md.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
#define NOISE_TAG "NOISE"
#define NOISE_LOGI(fmt, ...) ESP_LOGI(NOISE_TAG, fmt, ##__VA_ARGS__)
#else
#define NOISE_LOGI(fmt, ...) printf("[NOISE] " fmt "\n", ##__VA_ARGS__)
#endif

// --- Noise constants ---
#define NOISE_HASHLEN   32
#define NOISE_DHLEN     32
#define NOISE_KEYLEN    32
#define NOISE_PSKLEN    32
#define NOISE_NONCELEN  12
#define NOISE_TAGLEN    16

// --- Context definition (must match your header) ---
struct esph_noise_ctx {
    uint8_t ck[NOISE_HASHLEN];      // chaining key
    uint8_t h[NOISE_HASHLEN];       // handshake hash
    uint8_t send_key[NOISE_KEYLEN];
    uint8_t recv_key[NOISE_KEYLEN];
    uint64_t send_nonce;
    uint64_t recv_nonce;

    mbedtls_ctr_drbg_context drbg;
    mbedtls_entropy_context entropy;
    mbedtls_ecdh_context ecdh;      // ephemeral
    mbedtls_chachapoly_context chachapoly;
};

// --- Small helpers ---

static void hex_dump(const char *label, const uint8_t *buf, size_t len) {
    NOISE_LOGI("%s (%u bytes):", label, (unsigned)len);
    char line[3 * 16 + 1];
    size_t i = 0;
    while (i < len) {
        size_t chunk = (len - i > 16) ? 16 : (len - i);
        char *p = line;
        for (size_t j = 0; j < chunk; j++) {
            sprintf(p, "%02X ", buf[i + j]);
            p += 3;
        }
        *p = '\0';
        NOISE_LOGI("  %s", line);
        i += chunk;
    }
}

static int noise_sha256(uint8_t out[NOISE_HASHLEN],
                        const uint8_t *in, size_t in_len) {
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md) return -1;
    if (mbedtls_md(md, in, in_len, out) != 0)
        return -1;
    return 0;
}

static int noise_hkdf(const uint8_t ck[NOISE_HASHLEN],
                      const uint8_t *ikm, size_t ikm_len,
                      uint8_t out1[NOISE_HASHLEN],
                      uint8_t out2[NOISE_HASHLEN]) {
    // HKDF-Extract
    uint8_t prk[NOISE_HASHLEN];
    const mbedtls_md_info_t *md = mbedtls_md_info_from_type(MBEDTLS_MD_SHA256);
    if (!md) return -1;

    if (mbedtls_md_hmac(md, ck, NOISE_HASHLEN, ikm, ikm_len, prk) != 0)
        return -1;

    // HKDF-Expand for out1
    uint8_t t[NOISE_HASHLEN + 1];
    uint8_t counter = 1;
    if (mbedtls_md_hmac(md, prk, NOISE_HASHLEN, &counter, 1, t) != 0)
        return -1;
    memcpy(out1, t, NOISE_HASHLEN);

    // HKDF-Expand for out2
    uint8_t buf[NOISE_HASHLEN + 1 + NOISE_HASHLEN];
    memcpy(buf, t, NOISE_HASHLEN);
    buf[NOISE_HASHLEN] = 2;
    if (mbedtls_md_hmac(md, prk, NOISE_HASHLEN, buf, NOISE_HASHLEN + 1, out2) != 0)
        return -1;

    memset(prk, 0, sizeof(prk));
    memset(t, 0, sizeof(t));
    memset(buf, 0, sizeof(buf));
    return 0;
}

static int mix_hash(uint8_t h[NOISE_HASHLEN],
                    const uint8_t *data, size_t data_len) {
    uint8_t buf[NOISE_HASHLEN + 1024]; // enough for small messages
    if (data_len > sizeof(buf) - NOISE_HASHLEN)
        return -1;

    memcpy(buf, h, NOISE_HASHLEN);
    memcpy(buf + NOISE_HASHLEN, data, data_len);
    if (noise_sha256(h, buf, NOISE_HASHLEN + data_len) != 0)
        return -1;

    return 0;
}

static int mix_key(uint8_t ck[NOISE_HASHLEN],
                   const uint8_t *input_key_material, size_t ikm_len,
                   uint8_t out1[NOISE_HASHLEN],
                   uint8_t out2[NOISE_HASHLEN]) {
    if (noise_hkdf(ck, input_key_material, ikm_len, out1, out2) != 0)
        return -1;
    memcpy(ck, out1, NOISE_HASHLEN);
    return 0;
}

static void nonce_to_bytes(uint64_t nonce, uint8_t out[NOISE_NONCELEN]) {
    memset(out, 0, NOISE_NONCELEN);
    // last 8 bytes = little-endian nonce
    for (int i = 0; i < 8; i++) {
        out[NOISE_NONCELEN - 1 - i] = (uint8_t)((nonce >> (8 * i)) & 0xFF);
    }
}

static int encrypt_and_hash(esph_noise_ctx_t *ctx,
                            const uint8_t *key,
                            uint64_t *nonce_counter,
                            const uint8_t *plaintext, size_t plen,
                            uint8_t *out, size_t *out_len) {
    uint8_t nonce[NOISE_NONCELEN];
    nonce_to_bytes(*nonce_counter, nonce);
    (*nonce_counter)++;

    mbedtls_chachapoly_context chp;
    mbedtls_chachapoly_init(&chp);
    if (mbedtls_chachapoly_setkey(&chp, key) != 0) {
        mbedtls_chachapoly_free(&chp);
        return -1;
    }

    uint8_t tag[NOISE_TAGLEN];
    if (mbedtls_chachapoly_encrypt_and_tag(&chp,
                                           plen,
                                           nonce,
                                           ctx->h, NOISE_HASHLEN, // AD = h
                                           plaintext,
                                           out,
                                           tag) != 0) {
        mbedtls_chachapoly_free(&chp);
        return -1;
    }

    memcpy(out + plen, tag, NOISE_TAGLEN);
    *out_len = plen + NOISE_TAGLEN;

    // Mix ciphertext into h
    if (mix_hash(ctx->h, out, *out_len) != 0) {
        mbedtls_chachapoly_free(&chp);
        return -1;
    }

    mbedtls_chachapoly_free(&chp);
    NOISE_LOGI("Encrypted %u bytes (ChaCha20-Poly1305)", (unsigned)plen);
    hex_dump("cipher+tag", out, *out_len);
    return 0;
}

static int decrypt_and_hash(esph_noise_ctx_t *ctx,
                            const uint8_t *key,
                            uint64_t *nonce_counter,
                            const uint8_t *in, size_t in_len,
                            uint8_t *plaintext, size_t *plen) {
    if (in_len < NOISE_TAGLEN)
        return -1;

    size_t c_len = in_len - NOISE_TAGLEN;
    const uint8_t *cipher = in;
    const uint8_t *tag = in + c_len;

    uint8_t nonce[NOISE_NONCELEN];
    nonce_to_bytes(*nonce_counter, nonce);
    (*nonce_counter)++;

    mbedtls_chachapoly_context chp;
    mbedtls_chachapoly_init(&chp);
    if (mbedtls_chachapoly_setkey(&chp, key) != 0) {
        mbedtls_chachapoly_free(&chp);
        return -1;
    }

    if (mbedtls_chachapoly_auth_decrypt(&chp,
                                        c_len,
                                        nonce,
                                        ctx->h, NOISE_HASHLEN,
                                        tag,
                                        cipher,
                                        plaintext) != 0) {
        mbedtls_chachapoly_free(&chp);
        NOISE_LOGI("Decrypt failed: auth error");
        return -1;
    }

    *plen = c_len;

    // Mix ciphertext into h
    if (mix_hash(ctx->h, in, in_len) != 0) {
        mbedtls_chachapoly_free(&chp);
        return -1;
    }

    mbedtls_chachapoly_free(&chp);
    NOISE_LOGI("Decrypted %u bytes (ChaCha20-Poly1305)", (unsigned)*plen);
    return 0;
}

// --- Public API ---

int esph_noise_init(esph_noise_ctx_t *ctx, const char *psk_str) {
    memset(ctx, 0, sizeof(*ctx));

    mbedtls_entropy_init(&ctx->entropy);
    mbedtls_ctr_drbg_init(&ctx->drbg);
    mbedtls_ecdh_init(&ctx->ecdh);
    mbedtls_chachapoly_init(&ctx->chachapoly);

    const char *pers = "esph_noise";
    if (mbedtls_ctr_drbg_seed(&ctx->drbg,
                              mbedtls_entropy_func,
                              &ctx->entropy,
                              (const unsigned char *)pers,
                              strlen(pers)) != 0) {
        NOISE_LOGI("DRBG seed failed");
        return -1;
    }

    // Initialize h and ck with protocol name
    const char proto_name[] = "Noise_NNpsk0_25519_ChaChaPoly_SHA256";
    memset(ctx->h, 0, NOISE_HASHLEN);
    if (noise_sha256(ctx->h, (const uint8_t *)proto_name, sizeof(proto_name) - 1) != 0)
        return -1;
    memcpy(ctx->ck, ctx->h, NOISE_HASHLEN);

    NOISE_LOGI("Initialized Noise context");
    hex_dump("h (proto hash)", ctx->h, NOISE_HASHLEN);
    hex_dump("ck (chaining key)", ctx->ck, NOISE_HASHLEN);

    // Mix PSK (psk0)
    uint8_t psk[NOISE_PSKLEN];
    memset(psk, 0, sizeof(psk));
    size_t psk_len = strlen(psk_str);
    if (psk_len > NOISE_PSKLEN) psk_len = NOISE_PSKLEN;
    memcpy(psk, psk_str, psk_len);

    uint8_t temp1[NOISE_HASHLEN], temp2[NOISE_HASHLEN];
    if (mix_key(ctx->ck, psk, NOISE_PSKLEN, temp1, temp2) != 0)
        return -1;

    NOISE_LOGI("Mixed PSK (psk0) into chaining key");
    hex_dump("ck (after psk0)", ctx->ck, NOISE_HASHLEN);

    memset(psk, 0, sizeof(psk));
    memset(temp1, 0, sizeof(temp1));
    memset(temp2, 0, sizeof(temp2));

    ctx->send_nonce = 0;
    ctx->recv_nonce = 0;

    return 0;
}

// Client-side NNpsk0 handshake
int esph_noise_handshake(esph_noise_ctx_t *ctx, int sock) {
    int ret;
    uint8_t e_priv[NOISE_DHLEN];
    uint8_t e_pub[NOISE_DHLEN];

    // Generate ephemeral X25519 keypair
    mbedtls_ecp_group_id gid = MBEDTLS_ECP_DP_CURVE25519;
    if ((ret = mbedtls_ecdh_setup(&ctx->ecdh, gid)) != 0) {
        NOISE_LOGI("ecdh_setup failed: %d", ret);
        return -1;
    }

    if ((ret = mbedtls_ecdh_gen_public(&ctx->ecdh.grp,
                                       &ctx->ecdh.d,
                                       &ctx->ecdh.Q,
                                       mbedtls_ctr_drbg_random,
                                       &ctx->drbg)) != 0) {
        NOISE_LOGI("ecdh_gen_public failed: %d", ret);
        return -1;
    }

    // Export public key (X25519 is 32 bytes)
    size_t olen = 0;
    if ((ret = mbedtls_mpi_write_binary(&ctx->ecdh.Q.X, e_pub, NOISE_DHLEN)) != 0) {
        NOISE_LOGI("write_binary failed: %d", ret);
        return -1;
    }
    // Private scalar
    if ((ret = mbedtls_mpi_write_binary(&ctx->ecdh.d, e_priv, NOISE_DHLEN)) != 0) {
        NOISE_LOGI("write_binary(d) failed: %d", ret);
        return -1;
    }

    NOISE_LOGI("Generated ephemeral keypair");
    hex_dump("e_pub", e_pub, NOISE_DHLEN);

    // MixHash(e_pub)
    if (mix_hash(ctx->h, e_pub, NOISE_DHLEN) != 0)
        return -1;
    hex_dump("h (after e_pub)", ctx->h, NOISE_HASHLEN);

    // Send e_pub to server
    if (esph_transport_send(sock, e_pub, NOISE_DHLEN) != NOISE_DHLEN) {
        NOISE_LOGI("Failed to send e_pub");
        return -1;
    }
    NOISE_LOGI("Sent e_pub to server");

    // Receive re_pub from server
    uint8_t re_pub[NOISE_DHLEN];
    int rlen = esph_transport_recv(sock, re_pub, NOISE_DHLEN);
    if (rlen != NOISE_DHLEN) {
        NOISE_LOGI("Failed to receive re_pub");
        return -1;
    }
    NOISE_LOGI("Received re_pub from server");
    hex_dump("re_pub", re_pub, NOISE_DHLEN);

    // MixHash(re_pub)
    if (mix_hash(ctx->h, re_pub, NOISE_DHLEN) != 0)
        return -1;
    hex_dump("h (after re_pub)", ctx->h, NOISE_HASHLEN);

    // Compute DH(e, re)
    mbedtls_mpi re_x;
    mbedtls_mpi_init(&re_x);
    if ((ret = mbedtls_mpi_read_binary(&re_x, re_pub, NOISE_DHLEN)) != 0) {
        NOISE_LOGI("mpi_read_binary(re_pub) failed: %d", ret);
        mbedtls_mpi_free(&re_x);
        return -1;
    }

    // Put re into ctx->ecdh.Qp
    mbedtls_mpi_copy(&ctx->ecdh.Qp.X, &re_x);
    mbedtls_mpi_lset(&ctx->ecdh.Qp.Z, 1);
    mbedtls_mpi_lset(&ctx->ecdh.Qp.Y, 1); // not used for X25519

    uint8_t dh_out[NOISE_DHLEN];
    if ((ret = mbedtls_ecdh_compute_shared(&ctx->ecdh.grp,
                                           &ctx->ecdh.z,
                                           &ctx->ecdh.Qp,
                                           &ctx->ecdh.d,
                                           mbedtls_ctr_drbg_random,
                                           &ctx->drbg)) != 0) {
        NOISE_LOGI("ecdh_compute_shared failed: %d", ret);
        mbedtls_mpi_free(&re_x);
        return -1;
    }

    if ((ret = mbedtls_mpi_write_binary(&ctx->ecdh.z, dh_out, NOISE_DHLEN)) != 0) {
        NOISE_LOGI("write_binary(z) failed: %d", ret);
        mbedtls_mpi_free(&re_x);
        return -1;
    }

    NOISE_LOGI("Computed DH(e, re)");
    hex_dump("dh_out", dh_out, NOISE_DHLEN);

    // MixKey(DH)
    uint8_t temp1[NOISE_HASHLEN], temp2[NOISE_HASHLEN];
    if (mix_key(ctx->ck, dh_out, NOISE_DHLEN, temp1, temp2) != 0) {
        mbedtls_mpi_free(&re_x);
        return -1;
    }
    hex_dump("ck (after DH)", ctx->ck, NOISE_HASHLEN);

    // Derive final transport keys from ck
    // Noise NNpsk0: ck -> k1 (send), k2 (recv)
    memcpy(ctx->send_key, temp1, NOISE_KEYLEN);
    memcpy(ctx->recv_key, temp2, NOISE_KEYLEN);
    hex_dump("send_key", ctx->send_key, NOISE_KEYLEN);
    hex_dump("recv_key", ctx->recv_key, NOISE_KEYLEN);

    memset(temp1, 0, sizeof(temp1));
    memset(temp2, 0, sizeof(temp2));
    memset(dh_out, 0, sizeof(dh_out));
    mbedtls_mpi_free(&re_x);

    NOISE_LOGI("Noise handshake complete");
    return 0;
}

int esph_noise_encrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len) {
    return encrypt_and_hash(ctx,
                            ctx->send_key,
                            &ctx->send_nonce,
                            in, in_len,
                            out, out_len);
}

int esph_noise_decrypt(esph_noise_ctx_t *ctx,
                       const uint8_t *in, size_t in_len,
                       uint8_t *out, size_t *out_len) {
    return decrypt_and_hash(ctx,
                            ctx->recv_key,
                            &ctx->recv_nonce,
                            in, in_len,
                            out, out_len);
}
