#include<stdio.h>
#include<pthread.h>
#include<unistd.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
pthread_t tid1,tid2;
int num=1;
int buffer;
int done=0;
int fd[2];
int flag=0;
pthread_mutex_t lock;
void sig_handler(int sig)
{
	flag=1;
}
void *producer(void *arg)
{
	while(!flag)
	{
	    pthread_mutex_lock(&lock);
	    if(!done)
            {
		buffer=num++;
		done=1;
	    }
	    pthread_mutex_unlock(&lock);
	    usleep(1000000);
	}
	pthread_exit(NULL);
}
void *consumer(void *arg)
{
	while(!flag)
	{
		pthread_mutex_lock(&lock);
		if(done)
		{
			write(fd[1],&buffer,sizeof(buffer));
			done=0;
		}
		pthread_mutex_unlock(&lock);
	}
	pthread_exit(NULL);
}
int main()
{
	signal(SIGINT,sig_handler);
	pthread_mutex_init(&lock,NULL);
	if(pipe(fd)==-1)
	{
		perror("pipe");
		return 1;
	}
	pid_t pid=fork();
	printf("Process Id:%d\n",getpid());
	if(pid==0)
	{
		close(fd[0]);
		pthread_create(&tid1,NULL,producer,NULL);
		pthread_create(&tid2,NULL,consumer,NULL);
		pthread_join(tid1,NULL);
		pthread_join(tid2,NULL);
		close(fd[1]);
	}
	else
	{
		close(fd[1]);
		int number;
		while(!flag)
		{
	        	if(read(fd[0],&number,sizeof(number))>0)
				printf("Recieved numbers:%d\n",number);
		}
		printf("Exiting...");
		close(fd[0]);
	}
}
