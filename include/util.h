#pragma once
#include <stdint.h>
#define AXIOM_BLAKE2B_OUT_LEN 32
#define AXIOM_BLAKE2B_BLOCK_LEN 128
typedef struct {
    uint64_t h[8];
    uint64_t t;
    uint8_t  buf[AXIOM_BLAKE2B_BLOCK_LEN];
    size_t   buflen;
} axiom_blake2b_ctx;
void axiom_blake2b_init(axiom_blake2b_ctx *restrict ctx);
void axiom_blake2b_update(axiom_blake2b_ctx *restrict ctx,
    const void *restrict data, size_t len);
void axiom_blake2b_final(axiom_blake2b_ctx *restrict ctx,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN]);
int axiom_blake2b_file(const char *restrict path,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN]);
void axiom_hex_encode(const uint8_t *restrict in, size_t inlen,
    char *restrict out);
int axiom_hex_decode(const char *restrict in, uint8_t *restrict out,
    size_t outlen);
long long axiom_epoch_ns(void);
long long axiom_epoch_ns_pure(void) __attribute__((pure));
