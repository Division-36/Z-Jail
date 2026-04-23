#include <sys/socket.h>
#include <stdio.h>
int main(void){
    int fd=socket(AF_INET,SOCK_STREAM,0);
    if(fd<0)perror("socket");else printf("socket ok\n");
    return 0;
}
