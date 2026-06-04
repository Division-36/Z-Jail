#include <stdio.h>
#include <string.h>
#include "../include/util.h"
static const char *h_empty="0e5751c026e543b2e8ab2eb06099daa1d1e5df47778f7787faab45cdf12fe3a8";
static const char *h_abc="bddd813c634239723171ef3fee98579b94964e3bb1cb3e427262c8c068d52319";
static void hash(const char*l,const void*d,size_t n,const char*e){
    axiom_blake2b_ctx c;uint8_t o[32];char h[65];
    axiom_blake2b_init(&c);axiom_blake2b_update(&c,d,n);axiom_blake2b_final(&c,o);
    axiom_hex_encode(o,32,h);int ok=(strcmp(h,e)==0);
    printf("%-6s in-mem: %s  %s\n",l,h,ok?"OK":"MISMATCH");
    if(!ok)printf("       expected: %s\n",e);
}
int main(void){
    hash("empty","",0,h_empty);
    hash("abc","abc",3,h_abc);
    return 0;
}
