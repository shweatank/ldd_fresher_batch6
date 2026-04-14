#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
int main()
{
	pid_t pid=fork();
	if(pid==-1)
	{
		perror("fork");
		return 1;
	}
	if(pid>0)
	{
		printf("parent process id:%d\n",getpid());
		sleep(20);
	}
	else
	{
		printf("child process id:%d\n",getpid());
		printf("child exiting..\n");
		exit(0);
	}
}
