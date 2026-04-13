//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define IOCTL_MAGIC 'C'
#define IOCTL_SET_CASE_VALUE _IOWR(IOCTL_MAGIC,1,char[100])

int main(int argc,char*argv[])
{
	char str[100];
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	strcpy(str,argv[1]);
	printf("User :sending %s to kernel\n",str);
	ioctl(fd,IOCTL_SET_CASE_VALUE,str);
	printf("User: got back %s from kernel\n",str);
	close(fd);
	return 0;
}

