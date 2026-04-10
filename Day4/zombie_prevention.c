#include<stdio.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
void handler(int sig){
  while(waitpid(-1,NULL,WNOHANG)>0);
}
int main(){
 // signal(SIGCHLD,SIG_IGN);
//  signal(SIGCHLD,handler);
  pid_t pid=fork();
  if(pid==0){
    printf("child pid: %d\n",getpid());
    exit(0);

  }
  else{
    printf("Parent pid: %d\n",getpid());
    sleep(10);
    printf("Parent is awake and exiting\n");
    waitpid(-1,NULL,WNOHANG);
  //  wait(NULL);
  }
}
