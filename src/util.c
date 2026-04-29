#include "z_jail.h"
#include <time.h>

static const uint64_t IV[8] = {
    0x6a09e667f3bcc908ULL, 0xbb67ae8584caa73bULL,
    0x3c6ef372fe94f82bULL, 0xa54ff53a5f1d36f1ULL,
    0x510e527fade682d1ULL, 0x9b05688c2b3e6c1fULL,
    0x1f83d9abfb41bd6bULL, 0x5be0cd19137e2179ULL
};

static const uint8_t SIGMA[12][16] = {
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3},
    {11,8,12,0,5,2,15,13,10,14,3,6,7,1,9,4},
    {7,9,3,1,13,12,11,14,2,6,5,10,4,0,15,8},
    {9,0,5,7,2,4,10,15,14,1,11,12,6,8,3,13},
    {2,12,6,10,0,11,8,3,4,13,7,5,15,14,1,9},
    {12,5,1,15,14,13,4,10,0,7,6,3,9,2,8,11},
    {13,11,7,14,12,1,3,9,5,0,15,4,8,6,2,10},
    {6,15,14,9,11,3,0,8,12,2,13,7,1,4,10,5},
    {10,2,8,4,7,6,1,5,15,11,9,14,3,12,13,0},
    {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15},
    {14,10,4,8,9,15,13,6,1,12,0,2,11,7,5,3},
};

#define ROTR64(x, n) (((x) >> (n)) | ((x) << (64 - (n))))
#define G(v, a, b, c, d, x, y) \
    do { \
        v[a] = v[a] + v[b] + x; \
        v[d] = ROTR64(v[d] ^ v[a], 32); \
        v[c] = v[c] + v[d]; \
        v[b] = ROTR64(v[b] ^ v[c], 24); \
        v[a] = v[a] + v[b] + y; \
        v[d] = ROTR64(v[d] ^ v[a], 16); \
        v[c] = v[c] + v[d]; \
        v[b] = ROTR64(v[b] ^ v[c], 63); \
    } while (0)

static void b2b_compress(uint64_t h[8], const uint8_t block[128],
    uint64_t t, int last)
{
    uint64_t v[16], m[16];
    int i, r;

    for (i = 0; i < 8; i++)
        v[i] = h[i];
    for (i = 0; i < 8; i++)
        v[i + 8] = IV[i];

    v[12] ^= t;
    v[13] ^= 0;
    v[14] ^= last ? ~0ULL : 0;

    for (i = 0; i < 16; i++) {
        m[i] = (uint64_t)block[8*i]
             | (uint64_t)block[8*i+1] << 8
             | (uint64_t)block[8*i+2] << 16
             | (uint64_t)block[8*i+3] << 24
             | (uint64_t)block[8*i+4] << 32
             | (uint64_t)block[8*i+5] << 40
             | (uint64_t)block[8*i+6] << 48
             | (uint64_t)block[8*i+7] << 56;
    }

    for (r = 0; r < 12; r++) {
        G(v, 0, 4, 8, 12, m[SIGMA[r][0]],  m[SIGMA[r][1]]);
        G(v, 1, 5, 9, 13, m[SIGMA[r][2]],  m[SIGMA[r][3]]);
        G(v, 2, 6, 10, 14, m[SIGMA[r][4]], m[SIGMA[r][5]]);
        G(v, 3, 7, 11, 15, m[SIGMA[r][6]], m[SIGMA[r][7]]);
        G(v, 0, 5, 10, 15, m[SIGMA[r][8]],  m[SIGMA[r][9]]);
        G(v, 1, 6, 11, 12, m[SIGMA[r][10]], m[SIGMA[r][11]]);
        G(v, 2, 7, 8, 13,  m[SIGMA[r][12]], m[SIGMA[r][13]]);
        G(v, 3, 4, 9, 14,  m[SIGMA[r][14]], m[SIGMA[r][15]]);
    }

    for (i = 0; i < 8; i++)
        h[i] ^= v[i] ^ v[i + 8];
}

void axiom_blake2b_init(axiom_blake2b_ctx *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    ctx->h[0] = IV[0] ^ 0x0000000001010020ULL;
    ctx->h[1] = IV[1];
    ctx->h[2] = IV[2];
    ctx->h[3] = IV[3];
    ctx->h[4] = IV[4];
    ctx->h[5] = IV[5];
    ctx->h[6] = IV[6];
    ctx->h[7] = IV[7];
}

void axiom_blake2b_update(axiom_blake2b_ctx *ctx,
    const void *data, size_t len)
{
    const uint8_t *p = (const uint8_t *)data;
    size_t i;

    for (i = 0; i < len; i++) {
        ctx->buf[ctx->buflen++] = p[i];
        if (ctx->buflen == AXIOM_BLAKE2B_BLOCK_LEN) {
            b2b_compress(ctx->h, ctx->buf, ctx->t, 0);
            ctx->t += AXIOM_BLAKE2B_BLOCK_LEN;
            ctx->buflen = 0;
        }
    }
}

void axiom_blake2b_final(axiom_blake2b_ctx *ctx,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN])
{
    size_t i;

    if (ctx->buflen > 0) {
        memset(ctx->buf + ctx->buflen, 0,
            AXIOM_BLAKE2B_BLOCK_LEN - ctx->buflen);
    }
    b2b_compress(ctx->h, ctx->buf, ctx->t, 1);

    for (i = 0; i < 4; i++) {
        out[i*8+0] = (uint8_t)(ctx->h[i] >> 0);
        out[i*8+1] = (uint8_t)(ctx->h[i] >> 8);
        out[i*8+2] = (uint8_t)(ctx->h[i] >> 16);
        out[i*8+3] = (uint8_t)(ctx->h[i] >> 24);
        out[i*8+4] = (uint8_t)(ctx->h[i] >> 32);
        out[i*8+5] = (uint8_t)(ctx->h[i] >> 40);
        out[i*8+6] = (uint8_t)(ctx->h[i] >> 48);
        out[i*8+7] = (uint8_t)(ctx->h[i] >> 56);
    }
}

int axiom_blake2b_file(const char *path,
    uint8_t out[AXIOM_BLAKE2B_OUT_LEN])
{
    axiom_blake2b_ctx ctx;
    uint8_t buf[4096];
    int fd;
    ssize_t n;

    axiom_blake2b_init(&ctx);
    fd = open(path, O_RDONLY);
    if (fd < 0) return -1;

    while ((n = read(fd, buf, sizeof(buf))) > 0)
        axiom_blake2b_update(&ctx, buf, (size_t)n);
    close(fd);
    if (n < 0) return -1;

    axiom_blake2b_final(&ctx, out);
    return 0;
}

void axiom_hex_encode(const uint8_t *in, size_t inlen, char *out)
{
    static const char hex[] = "0123456789abcdef";
    size_t i;
    for (i = 0; i < inlen; i++) {
        out[i*2]   = hex[(in[i] >> 4) & 0xf];
        out[i*2+1] = hex[in[i] & 0xf];
    }
    out[inlen * 2] = '\0';
}

int axiom_hex_decode(const char *in, uint8_t *out, size_t outlen)
{
    size_t i;
    for (i = 0; i < outlen; i++) {
        int hi, lo;
        if (in[i*2] >= '0' && in[i*2] <= '9') hi = in[i*2] - '0';
        else if (in[i*2] >= 'a' && in[i*2] <= 'f') hi = in[i*2] - 'a' + 10;
        else if (in[i*2] >= 'A' && in[i*2] <= 'F') hi = in[i*2] - 'A' + 10;
        else return -1;
        if (in[i*2+1] >= '0' && in[i*2+1] <= '9') lo = in[i*2+1] - '0';
        else if (in[i*2+1] >= 'a' && in[i*2+1] <= 'f') lo = in[i*2+1] - 'a' + 10;
        else if (in[i*2+1] >= 'A' && in[i*2+1] <= 'F') lo = in[i*2+1] - 'A' + 10;
        else return -1;
        out[i] = (uint8_t)((hi << 4) | lo);
    }
    return 0;
}

long long axiom_epoch_ns(void)
{
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) < 0) return -1;
    return (long long)ts.tv_sec * 1000000000LL + (long long)ts.tv_nsec;
}

AXIOM_STATIC_ASSERT(sizeof(axiom_blake2b_ctx) <= 256, blake2b_ctx_size);

