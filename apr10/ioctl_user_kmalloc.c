//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
struct buffer
{
	int n;
	char *str;
};
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct buffer)

int main(int argc,char*argv[])
{
	struct buffer data;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	data.str=malloc(100);
	scanf("%s",data.str);
	data.n=strlen(data.str)+1;
	printf("User :sending %s to kernel\n",data.str);
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("User: got back %s from kernel\n",data.str);
	free(data.str);
	close(fd);
	return 0;
}

