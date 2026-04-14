#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<pthread.h>
#include<string.h>
#include<unistd.h>

#define CHUNKSIZE 100

typedef struct
{
	pthread_t tid;
	int file_fd;
	long offset;
}Threaddata;
int fd[2];
int fi_fd;
int flag=0;
void sig_handler(int sig)
{
	flag=1;
}
void *fun(void *arg)
{
	Threaddata *Data=(Threaddata *)arg;
	char buf[100];
	size_t n;
	while(!flag)
	{
		if((n=pread(Data->file_fd,buf,sizeof(buf),Data->offset))>0)
		{
			write(fd[1],buf,n);
		}
		sleep(10);
	}
	pthread_exit(NULL);
}
int main(int argc,char *argv[])
{
	if(argc!=3)
	{
		puts("Insufficent input");
		return 0;
	}
	signal(SIGTERM,sig_handler);
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
	fi_fd=open(argv[2],O_RDONLY);
	if(fi_fd==-1)
	{
		perror("file failed");
		return 1;
	}
	if(pid==0)
	{
		close(fd[0]);
		int i;
		int threads_count=atoi(argv[1]);
		pthread_t tid[threads_count];
		for(i=0;i<threads_count;i++)
		{
			Threaddata *Data=malloc(sizeof(Threaddata));
			Data->tid=i;
			Data->file_fd=fi_fd;
			Data->offset=CHUNKSIZE*i;
			pthread_create(&tid[i],NULL,fun,Data);
		}
		for(i=0;i<threads_count;i++)
		{
			pthread_join(tid[i],NULL);
		}
		close(fd[1]);
	}
	else
	{
		close(fd[1]);
		char buf[100];
		size_t n;
		while((n=read(fd[0],buf,sizeof(buf)))>0)
		{
			write(STDOUT_FILENO,buf,n);
			write(STDOUT_FILENO,"\n",1);
		}
		close(fd[0]);
	}
}
