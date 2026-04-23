#include <unistd.h>
#include <stdio.h>
int main(void){
    if(chroot("/")<0){perror("chroot1");return 1;}
    if(chroot(".")<0){perror("chroot2");return 1;}
    printf("double chroot ok\n");
    return 0;
}
