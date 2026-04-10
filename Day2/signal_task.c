#include<stdio.h>
#include<unistd.h>
#include<signal.h>
void signalhander(int sig) {
    printf("\n[Signal %d] Ctrl+Z pressed!!!!\n", sig);
}

int main() {
    signal(SIGTSTP, signalhander);
   while(1){
    printf("pid : %d\n",getpid());
    sleep(4);
   }
}
