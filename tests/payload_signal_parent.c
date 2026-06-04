#include <signal.h>
#include <stdio.h>
#include <unistd.h>
int main(void){
    union sigval v;v.sival_int=42;
    if(sigqueue(getppid(),SIGUSR1,v)<0)perror("sigqueue");
    else printf("sigqueue ok\n");
    return 0;
}
