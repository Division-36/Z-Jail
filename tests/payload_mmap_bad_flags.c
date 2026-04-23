#include <sys/mman.h>
#include <stdio.h>
int main(void){
    void*p=mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS,-1,0);
    if(p==MAP_FAILED){perror("mmap");return 1;}
    printf("mmap MAP_SHARED ok\n");
    return 0;
}
