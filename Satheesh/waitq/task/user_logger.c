#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
#include<string.h>
int main()
{
	int fd=open("/dev/waitq_basic",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char tmp[20];
	int n=1;
	while(1)
	{
		sprintf(tmp,"Log %d",n);
		write(fd,tmp,strlen(tmp));
		printf("From Kernel :\n");
		sleep(1);

		n++;
	}
}
