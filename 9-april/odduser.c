//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
#define IOCTL_GET_VALUE _IOWR(IOCTL_MAGIC,2,char *)
int main(void)
{
	int fd;
	int value;
	char str[10];
	scanf("%d",&value);
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("user: sending %d to kernel\n",value);
	ioctl(fd,IOCTL_SET_VALUE,&value);
	ioctl(fd,IOCTL_GET_VALUE,str);
	printf("user: got back %s from kernel\n",str);
	close(fd);
	return 0;
}

