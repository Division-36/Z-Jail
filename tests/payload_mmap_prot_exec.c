#include <sys/mman.h>
#include <stdio.h>
int main(void){
    void*p=mmap(NULL,4096,PROT_READ|PROT_WRITE|PROT_EXEC,MAP_PRIVATE|MAP_ANONYMOUS,-1,0);
    if(p==MAP_FAILED){perror("mmap");return 1;}
    printf("mmap PROT_EXEC succeeded\n");
    return 0;
}
