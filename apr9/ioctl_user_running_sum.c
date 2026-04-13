//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<stdlib.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)

int main(int argc,char*argv[])
{
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	int n1=atoi(argv[1]);
	printf("User :sending %d to kernel\n",n1);
	ioctl(fd,IOCTL_SET_VALUE,&n1);
	printf("User: got back %d from kernel\n",n1);
	close(fd);
	return 0;
}

