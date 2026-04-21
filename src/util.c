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
