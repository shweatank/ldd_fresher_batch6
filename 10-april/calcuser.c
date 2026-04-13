//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#define IOCTL_MAGIC 'A'
struct user
{
	int a;
	int b;
	int res;
	char str[10];
};
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct user)
int main(int argc,char * argv[])
{
	struct user data;
	data.a=atoi(argv[1]);
	data.b=atoi(argv[2]);
	strcpy(data.str,argv[3]);
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("user: sending %d %d %s to kernel\n",data.a,data.b,data.str);
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("user: got back %d from kernel\n",data.res);
	close(fd);
	return 0;
}
