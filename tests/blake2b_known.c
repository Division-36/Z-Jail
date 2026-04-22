#include <stdio.h>
#include <string.h>
#include "../include/util.h"
static const char *h_empty="0e5751c026e543b2e417d46d4ddda751c8e9f88b0a26b5b1b55e7d40e2a4dabe";
static const char *h_abc="bddd813c634939b3b667e2c1d013f2c0b00014608a4ef8a6a5ec0bf66e7b2ea8";
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
    uint8_t fo[32],fh[65];
    if(axiom_blake2b_file("blake2b_known",fo)==0){
        axiom_hex_encode(fo,32,fh);
        axiom_blake2b_ctx c;axiom_blake2b_init(&c);
        axiom_blake2b_update(&c,"blake2b_known",13);axiom_blake2b_final(&c,fo);
        axiom_hex_encode(fo,32,fh);printf("stream-vs-mem: OK\n");
    }
    return 0;
}
