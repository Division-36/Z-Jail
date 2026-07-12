#include <unistd.h>
#include <stdio.h>
int main(void){
    char*argv[]={"/bin/target",NULL};
    execve("/bin/target",argv,NULL);
    perror("execve");
    return 1;
}
