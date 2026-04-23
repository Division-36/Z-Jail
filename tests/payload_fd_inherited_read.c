#include <unistd.h>
#include <stdio.h>
int main(void){
    char buf[64];
    ssize_t n=read(0,buf,sizeof(buf));
    if(n<0){perror("read");return 1;}
    write(1,buf,(size_t)n);
    return 0;
}
