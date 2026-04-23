#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
int main(void){
    void*p=mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(p==MAP_FAILED){perror("mmap");return 1;}
    memset(p,0xAA,128);printf("mmap allowed flags ok\n");
    return 0;
}
