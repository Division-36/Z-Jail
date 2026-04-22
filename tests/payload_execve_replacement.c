#include <unistd.h>
#include <stdio.h>
int main(void){
    char*argv[]={"/bin/false",NULL};
    execve("/bin/false",argv,NULL);
    perror("execve");
    return 1;
}
