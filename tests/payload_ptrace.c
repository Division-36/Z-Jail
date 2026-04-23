#include <sys/ptrace.h>
#include <stdio.h>
int main(void){
    long r=ptrace(PTRACE_TRACEME,0,NULL,NULL);
    if(r<0)perror("ptrace");else printf("ptrace ok\n");
    return 0;
}
