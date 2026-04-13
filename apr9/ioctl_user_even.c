//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define IOCTL_MAGIC 'D'
#define IOCTL_SET_EVEN_VALUE _IOWR(IOCTL_MAGIC,1,int)

int main(int argc,char *argv[])
{
	int fd;

	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	int value=atoi(argv[1]);
	printf("User :sending %d to kernel\n",value);
	ioctl(fd,IOCTL_SET_EVEN_VALUE,&value);
	if(value==1)
	{
		printf("User: got back from kernel is Even\n");
	}
	else if(value==0)
	{
		printf("User: got back from kernel is Odd\n");
	}
	close(fd);
	return 0;
}

