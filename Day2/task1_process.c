#include<stdio.h>
#include<sys/wait.h>
#include<unistd.h>
int main(){
  pid_t pid1,pid2;
  pid1=fork();
  char data[100];
  while(1){
  if(pid1==0){
      printf("Enter data\n");
      scanf("%s",data);
  }
  else{
    wait(NULL);
  }
  if(pid2==0){
      printf("%s\n",data);
  }
  else{
    wait(NULL);
  }
  }
}
