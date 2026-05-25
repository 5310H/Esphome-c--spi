#ifndef CRYPTO_FAKE_H
#define CRYPTO_FAKE_H

#include <sodium.h>
#include <stdint.h>
#include <stdbool.h>

static const uint8_t FAKE_KEY[crypto_aead_chacha20poly1305_ietf_KEYBYTES] = {
    1,2,3,4,5,6,7,8,
    9,10,11,12,13,14,15,16,
    17,18,19,20,21,22,23,24,
    25,26,27,28,29,30,31,32
};

static inline void fake_nonce(uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES],
                              uint64_t counter)
{
    memset(nonce, 0, crypto_aead_chacha20poly1305_ietf_NPUBBYTES);
    memcpy(nonce + 4, &counter, sizeof(counter)); // simple, deterministic
}

static inline bool fake_encrypt(const uint8_t *plaintext, size_t plen,
                                uint8_t *ciphertext, size_t *clen,
                                uint64_t counter)
{
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES];
    fake_nonce(nonce, counter);

    unsigned long long outlen = 0;
    if (crypto_aead_chacha20poly1305_ietf_encrypt(
            ciphertext, &outlen,
            plaintext, plen,
            NULL, 0, NULL,
            nonce, FAKE_KEY) != 0) {
        return false;
    }
    *clen = (size_t)outlen;
    return true;
}

static inline bool fake_decrypt(const uint8_t *ciphertext, size_t clen,
                                uint8_t *plaintext, size_t *plen,
                                uint64_t counter)
{
    uint8_t nonce[crypto_aead_chacha20poly1305_ietf_NPUBBYTES];
    fake_nonce(nonce, counter);

    unsigned long long outlen = 0;
    if (crypto_aead_chacha20poly1305_ietf_decrypt(
            plaintext, &outlen,
            NULL,
            ciphertext, clen,
            NULL, 0,
            nonce, FAKE_KEY) != 0) {
        return false;
    }
    *plen = (size_t)outlen;
    return true;
}

#endif
