#include<stdio.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#define MAGIC_NUMBER 'B'
#define LED_SEND _IOW(MAGIC_NUMBER,1,int)
int main()
{
	int ch;
	int fd=open("/dev/ioctl_led",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return fd;
	}
	while(1)
	{
		printf("Enter operation\n");
		scanf("%d",&ch);
		ioctl(fd,LED_SEND,&ch);
	}

}
