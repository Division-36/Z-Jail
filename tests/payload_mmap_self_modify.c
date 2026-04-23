#include <sys/mman.h>
#include <stdio.h>
int main(void){
    void*p=mmap(NULL,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(p==MAP_FAILED){perror("mmap wx");return 1;}
    ((unsigned char*)p)[0]=0xC3;
    __asm__ volatile("call *%0"::"r"(p):"memory");
    printf("self-modify ok\n");
    return 0;
}
