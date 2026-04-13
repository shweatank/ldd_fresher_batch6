//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
struct data
{
	int a;
	int b;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct data)
#define IOCTL_GET_VALUE _IOWR(IOCTL_MAGIC,2,int)

int main(void)
{
	int fd;
	struct data data;
	int value;
	data.a=20;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("user: sending %d to kernel\n",data.a);
	ioctl(fd,IOCTL_SET_VALUE,&data);
	ioctl(fd,IOCTL_GET_VALUE,&value);
	printf("user: got back %d from kernel\n",value);
	close(fd);
	return 0;
}
