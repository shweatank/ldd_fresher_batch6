#include<stdio.h>
#include<sys/types.h>
#include<signal.h>

int main(){
int pid;
printf("Enter the pid: ");
scanf("%d",&pid);
kill(pid,2);
}
