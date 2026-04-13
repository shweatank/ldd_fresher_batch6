#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<pthread.h>
#include<sys/types.h>
#include<stdlib.h>

int fd1[2]; //fd buffer for pipe of size 2 (read,write ends)

volatile sig_atomic_t paused=0;

void pause_handler(int signum){//signal handler to haandle PAUSE SIGUSR1
paused=1;
printf("Paused the process!\n");
}

void resume_handler(int signum){//signal handler to haandle RESUME SIGUSR2
paused=0;
printf("Resumed the process!\n");
}

void* producer(void* arg){

int n=1;

while(1){
if(!paused){//if pause signal not passed write to pipe
write(fd1[1],&n,sizeof(n));
printf("Sent: %d\n",n);
n++;
sleep(1);
}
else{
usleep(1000);//busy wait
}
}

pthread_exit(NULL);
}

int main(){

if(pipe(fd1)==-1){
perror("pipe");
return 1;
}

int pid=fork();

if(pid<0){
perror("fork");
return 1;
}

else if(pid==0){
close(fd1[0]); //close read end

signal(SIGUSR1,pause_handler);//register signL to pause
signal(SIGUSR2,resume_handler);//register signal to resume

pthread_t t; //create a thread variable

pthread_create(&t,NULL,producer,NULL);
pthread_join(&t,NULL);

close(fd1[1]);
}

else{
close(fd1[1]);//close write end
int data;

printf("Parent PID: %d\n",getpid());
printf("kill -SIGUSR1  %d (pause)\n",pid);
printf("kill -SIGUSR2 %d(resume)\n",pid);
//send signals via another terminal kill -SIGUSR1 PID (or) -SIGUSR2 PID 
while(1){
if(read(fd1[0],&data,sizeof(data))>0){
printf("Recieved: %d\n",data);
}
}

close(fd1[0]);
}

}

