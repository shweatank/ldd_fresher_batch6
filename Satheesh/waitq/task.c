#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	int fd=open("/dev/waitq_basic",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char tmp[100]="Hello Im coming from user";
	while(1)
	{
		write(fd,tmp,strlen(tmp));
		printf("From Kernel :%s\n",tmp);
		sleep(1);
	}
}
