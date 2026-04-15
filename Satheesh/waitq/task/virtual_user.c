#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<stdlib.h>
int main()
{
	int fd=open("/dev/waitq_basic",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
//	char tmp[20];
	int n;
	while(1)
	{
		read(fd,&n,sizeof(int));
		printf("From Kernel :%d\n",n);
		sleep(1);
	}
}
