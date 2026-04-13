//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<stdlib.h>
#include <string.h>
struct rev
{
	char str[10];
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_REV_VALUE _IOWR(IOCTL_MAGIC,1,struct rev)

int main(int argc,char*argv[])
{
	struct rev data;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	strcpy(data.str,argv[1]);
	printf("%s\n",data.str);
	printf("User :sending %s to kernel\n",data.str);
	ioctl(fd,IOCTL_SET_REV_VALUE,&data);
	printf("User: got back %s from kernel\n",data.str);
	close(fd);
	return 0;
}


