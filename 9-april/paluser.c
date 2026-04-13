//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char *)
int main(void)
{
	int fd;
	char str[100];
	scanf("%s",str);
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("user: sending %s to kernel\n",str);
	ioctl(fd,IOCTL_SET_VALUE,str);
	printf("user: got back %s from kernel\n",str);
	close(fd);
	return 0;
}

