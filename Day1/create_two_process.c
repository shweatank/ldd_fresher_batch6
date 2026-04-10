#include<stdio.h>
#include<stdlib.h>
#include<sys/wait.h>
#include<unistd.h>
int main(int argc,char *argv[]){
  if(argc!=3){
    puts("a.out input1 input2\n"); return 0;
  }
  pid_t pid;
  pid=fork();
  if(pid<0){
    printf("fork call failed\n"); return 0;
  }
  else if(pid==0){
    execlp("expr","expr",argv[1] ,"+",argv[2],(char*)NULL);
    perror("exec failed");
    exit(1);
  }
  else{
    wait(NULL);	  
    printf("Child called\n");
  }
}
