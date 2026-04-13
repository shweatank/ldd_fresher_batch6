#include<stdio.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<pthread.h>
#include<sys/types.h>
#include<stdlib.h>

int fd1[2]; //fd buffer for pipe of size 2 (read,write ends)

volatile sig_atomic_t stop=0;

int buffer;//share data between threads
int flag=0;//full-1, emty-0

pthread_mutex_t lock; //only one thread access buffer
pthread_cond_t cond;


void signal_handler(int signum){//signal handler to haandle ctrl+C (SIGINT)
stop=1;
pthread_cond_broadcast(&cond); //wakeup all waiting threads once stop signal is received so that they no more wait, for safer use place it inside producer and consumer once stop detected
}

void* producer(void* arg){
int n=1;

while(!stop){
pthread_mutex_lock(&lock);// no other thread can aceess bufffer

while(flag==1 && !stop)
pthread_cond_wait(&cond,&lock);//wait till buffer  becomes empty , mutex released, sleep till consumer signals

if(stop){
pthread_mutex_unlock(&lock);
break;
}

buffer=n++;
printf("Produced: %d\n",buffer);

flag=1;

pthread_cond_signal(&cond); //signal consumer if waiting

pthread_mutex_unlock(&lock);//unlock

sleep(1);//slow down execution
}

pthread_exit(NULL);
}

void* consumer(void* arg){
int data;

while(!stop){
pthread_mutex_lock(&lock);

while(flag==0 && !stop)// wait till producer sends signal that buffer full
pthread_cond_wait(&cond,&lock);

if(stop){
pthread_mutex_unlock(&lock);
break;
}

data=buffer;

flag=0; //as consumed, buffer is empty
 write(fd1[1],&data,sizeof(data));

pthread_cond_signal(&cond);
pthread_mutex_unlock(&lock);
}
pthread_exit(NULL);

}

 
int main(){

//creating pipe 
if(pipe(fd1)==-1){
perror("pipe");
return 1;
}

signal(SIGINT,signal_handler);

int pid = fork();

if(pid<0){
perror("fork");
return 1;//failure in OS
}

else if(pid==0){ //child process

close(fd1[0]); // clse read end as chile writes

pthread_t t1,t2; //t1-producer thread variable and t2 consumer thread variable

pthread_mutex_init(&lock,NULL);//only one thread access buffer at a time
pthread_cond_init(&cond,NULL); //creates conditonal variable for waiting and signaling between threads

pthread_create(&t1,NULL,producer,NULL);//t1- thread id, null-default attribute, producer-function to run, null- arguement to function
pthread_create(&t2,NULL,consumer,NULL);//consumer - function to run

pthread_join(t1,NULL);//wait for producer to finish
pthread_join(t2,NULL);//wait for consumer to finish(main thread waits)

close(fd1[1]);
printf("Child exited!\n");
}

else{ //parent process
int received_data;
close(fd1[1]);

while(!stop){
if(read(fd1[0],&received_data,sizeof(received_data))>0){
printf("Parent received: %d\n",received_data);
}
}
close(fd1[0]);
printf("Parent exited!\n");
}
}
