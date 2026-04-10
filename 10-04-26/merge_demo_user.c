//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)

int main(void)
{
	int fd;
	int res=20;

	fd=open("/dev/merge_demo_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	if(ioctl(fd,IOCTL_SET_VALUE,&res)<0)
	{
		perror("ioctl");
		return 1;
	}
	if(ioctl(fd,IOCTL_GET_VALUE,&res)<0)
	{
		perror("ioctl");
		return 1;
	}
	printf("The result is %d\n",res);

	close(fd);
	return 0;
}
