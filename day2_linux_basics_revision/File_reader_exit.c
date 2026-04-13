#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<fcntl.h>
#include<string.h>
#include<signal.h>

#define THREAD_COUNT 3
#define BUF_SIZE 100

int fd; 
int pipe_fd[2];
volatile sig_atomic_t stop=0;

typedef struct{
int start;
int size;
}thread_data;

//signal handler
void handle_sigterm(int sig)
{
stop=1;
}

//thread function
void* read_file(void *arg)
{
thread_data *data=(thread_data*)arg;
char buffer[BUF_SIZE];

lseek(fd,data->start,SEEK_SET);
int bytes_to_read=data->size;
while(bytes_to_read>0 && !stop)
{
int chunk=(bytes_to_read >BUF_SIZE)? BUF_SIZE:bytes_to_read;
int n=read(fd,buffer,chunk);
if(n<=0)
break;

write(pipe_fd[1],buffer,n);
bytes_to_read -= n;
sleep(1); //slow down to observe signal effect
}
pthread_exit(NULL);
}

int main(){
pipe(pipe_fd);
pid_t pid=fork();
if(pid>0)
{
//parent
printf("Child pid: %d\n",pid);
close(pipe_fd[1]);

char buffer[BUF_SIZE];

while(1)
{
int n=read(pipe_fd[0],buffer,BUF_SIZE);
if(n<=0)
break;
write(STDOUT_FILENO,buffer,n);
}
close(pipe_fd[0]);
}
else
{
//child
signal(SIGTERM,handle_sigterm);
fd=open("input.txt",O_RDONLY);
if(fd<0)
{
perror("File oen error");
exit(1);
}

int file_size=lseek(fd,0,SEEK_END);
int part=file_size/THREAD_COUNT;
pthread_t threads[THREAD_COUNT];
thread_data tdata[THREAD_COUNT];

close(pipe_fd[0]);

for(int i=0;i<THREAD_COUNT;i++)
{
tdata[i].start=i*part;
tdata[i].size=(i==THREAD_COUNT -1)?(file_size-i*part):part;
pthread_create(&threads[i],NULL,read_file,&tdata[i]);
}
for(int i=0;i<THREAD_COUNT;i++)
{
pthread_join(threads[i],NULL);
}
close(fd);
close(pipe_fd[1]);
exit(0);
}
return 0;
}

