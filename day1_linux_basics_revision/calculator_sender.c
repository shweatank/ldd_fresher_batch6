#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<stdlib.h>

int main(int argc, char *argv[]){
if(argc<3){
printf("Insufficient arguements!\n");
return 0;
}
 int pid=atoi(argv[1]);
int signum=atoi(argv[2]);
kill(pid,signum);
}
