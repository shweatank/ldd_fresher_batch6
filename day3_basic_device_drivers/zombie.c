#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/wait.h>
int main(){

int pid=fork();
if(pid==0){
printf("Child pid: %d\n",getpid());
printf("Child is exiting>>\n");
exit(0);
}
else if(pid>0){
printf("Parent pid: %d\n",getpid());
printf("I am parent!\n");
wait(NULL);
sleep(30); 
}
else{
printf("Fork failed\n");
}
}
