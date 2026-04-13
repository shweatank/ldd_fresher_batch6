#include<stdio.h>
#include <signal.h>
#include<unistd.h>
int signal_handler(int signal){
printf("HEllo\n");
}

int main(){
printf("get pid: %d\n",getpid());
signal(2,signal_handler);
signal(2,SIG_IGN);
return 0;
}

/*int main(){
printf("get pid: %d\n",getpid());
signal(2,SIG_DFL);
while(1);
return 0;
}*/
