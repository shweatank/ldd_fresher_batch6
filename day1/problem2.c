#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>
pthread_t tid1;
int flag=0;
int num=1;
int fd[2];
void sig_handler(int sig)
{
	if(sig==SIGUSR1)
		flag=1;
	else if(sig==SIGUSR2)
		flag=0;
}
void *fun()
{
	while(1)
	{
		if(!flag)
		{
			write(fd[1],&num,sizeof(num));
			num++;
			sleep(1);
		}
	}
	pthread_exit(NULL);
}
int main()
{
	signal(SIGUSR1,sig_handler);
	signal(SIGUSR2,sig_handler);
	if(pipe(fd)==-1)
	{
		perror("pipe");
		return 1;
	}
	pid_t pid=fork();
	if(pid==-1)
	{
		perror("fork");
		return 1;
	}
	printf("process id:%d\n",getpid());
	if(pid==0)
	{
		//child process
		close(fd[0]);
		pthread_create(&tid1,NULL,fun,NULL);
		pthread_join(tid1,NULL);
		close(fd[1]);
	}
	else
	{
		//parent process
		close(fd[1]);
		int value;
		while(1)
		{
			if(read(fd[0],&value,sizeof(value))>0)
				printf("Received number:%d\n",value);
		}
		close(fd[0]);
	}
}
