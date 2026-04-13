//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<stdlib.h>

struct cal
{
	int n1;
	int n2;
	int res;
	char op;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct cal)

int main(int argc,char*argv[])
{
	struct cal data;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	data.n1=atoi(argv[1]);
	data.op=argv[2][0];
	printf("%c\n",data.op);
	data.n2=atoi(argv[3]);
	printf("User :sending %d %c %d to kernel\n",data.n1,data.op,data.n2);
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("User: got back %d from kernel\n",data.res);
	close(fd);
	return 0;
}

