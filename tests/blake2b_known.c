#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../include/util.h"
static const char *h_empty="0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8";
static const char *h_abc="bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319";
/* Multi-block known-answer vectors (BLAKE2b-256, verified against b2sum -l 256).
 * These exercise the block-boundary logic that a single-block-only KAT misses:
 * exact block boundary (128), just over it (129), and several full blocks. */
static const char *h_zero128="378d0caaaa3855f1b38693c1d6ef004fd118691c95c959d4efa950d6d6fcf7c1";
static const char *h_zero129="baadfb64c3bd2cd187b54accc5e61a0720ed86bf48c28017873536cf9015d1b8";
static const char *h_a200="6b6e59aaf00eb730cf93de53560846722184bbd92f8368c21ffa95380c2f9fe6";
static const char *h_seq256="39a7eb9fedc19aabc83425c6755dd90e6f9d0c804964a1f4aaeea3b9fb599835";
static const char *h_lorem="60d7615954339cb409f023ebba338d1f81f1aa5ca4887932662571fccac9a178";
static int failures=0;
static void hash(const char*l,const void*d,size_t n,const char*e){
    axiom_blake2b_ctx c;uint8_t o[32];char h[65];
    axiom_blake2b_init(&c);axiom_blake2b_update(&c,d,n);axiom_blake2b_final(&c,o);
    axiom_hex_encode(o,32,h);int ok=(strcmp(h,e)==0);
    printf("%-8s in-mem: %s  %s\n",l,h,ok?"OK":"MISMATCH");
    if(!ok){printf("         expected: %s\n",e);failures++;}
}
/* Feed the same data one byte at a time to prove streaming across many
 * update() calls produces the identical digest as a single update(). */
static void hash_streamed(const char*l,const void*d,size_t n,const char*e){
    axiom_blake2b_ctx c;uint8_t o[32];char h[65];const uint8_t*p=(const uint8_t*)d;size_t i;
    axiom_blake2b_init(&c);
    for(i=0;i<n;i++)axiom_blake2b_update(&c,p+i,1);
    axiom_blake2b_final(&c,o);
    axiom_hex_encode(o,32,h);int ok=(strcmp(h,e)==0);
    printf("%-8s stream: %s  %s\n",l,h,ok?"OK":"MISMATCH");
    if(!ok){printf("         expected: %s\n",e);failures++;}
}
int main(void){
    unsigned char zero[129];unsigned char a200[200];unsigned char seq256[256];
    static const char lorem_unit[]="The quick brown fox jumps over the lazy dog. ";
    char lorem[451];int i;
    memset(zero,0,sizeof(zero));
    memset(a200,'a',sizeof(a200));
    for(i=0;i<256;i++)seq256[i]=(unsigned char)i;
    lorem[0]='\0';for(i=0;i<10;i++)strcat(lorem,lorem_unit);
    hash("empty","",0,h_empty);
    hash("abc","abc",3,h_abc);
    hash("zero128",zero,128,h_zero128);
    hash("zero129",zero,129,h_zero129);
    hash("a200",a200,200,h_a200);
    hash("seq256",seq256,256,h_seq256);
    hash("lorem",lorem,strlen(lorem),h_lorem);
    hash_streamed("zero129",zero,129,h_zero129);
    hash_streamed("a200",a200,200,h_a200);
    hash_streamed("seq256",seq256,256,h_seq256);
    hash_streamed("lorem",lorem,strlen(lorem),h_lorem);
    if(failures){printf("FAIL: %d vector(s) mismatched\n",failures);return 1;}
    printf("all vectors OK\n");
    return 0;
}
