#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
int main()
{
	int fd=open("/dev/basic_waitq",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char tmp[20];
	while(1)
	{
		read(fd,tmp,sizeof(tmp));
		printf("From Kernel :%s\n",tmp);
		sleep(1);
	}
}
