#include <sys/mount.h>
#include <stdio.h>
int main(void){
    if(mount("/","/",NULL,MS_BIND,NULL)<0)perror("mount");
    else printf("mount ok\n");
    return 0;
}
