#include <unistd.h>
#include <stdio.h>
int main(void){
    if(chroot("/")<0)perror("chroot");
    else printf("chroot succeeded\n");
    return 0;
}
