#pragma once

#define AXIOM_BLAKE2B_OUT_LEN 32
#define AXIOM_BLAKE2B_BLOCK_LEN 128

typedef struct {
    uint64_t h[8];
    uint64_t t;
    uint8_t  buf[AXIOM_BLAKE2B_BLOCK_LEN];
    size_t   buflen;
} axiom_blake2b_ctx;

void axiom_blake2b_init(axiom_blake2b_ctx *ctx);
void axiom_blake2b_update(axiom_blake2b_ctx *ctx,
    const void *data, size_t len);
void axiom_blake2b_final(axiom_blake2b_ctx *ctx,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN]);

int axiom_blake2b_file(const char *path,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN]);

void axiom_hex_encode(const uint8_t *in, size_t inlen, char *out);
int axiom_hex_decode(const char *in, uint8_t *out, size_t outlen);
long long axiom_epoch_ns(void);
