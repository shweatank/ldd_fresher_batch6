//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include<stdlib.h>
struct average
{
	int sum;
	int avg;
	int n;
	int arr[100];
};

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct average)

int main(int argc,char*argv[])
{
	struct average data;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	int n;
	scanf("%d",&n);
	data.n=n;
	for (int i=0; i<n; i++)
	{
		scanf("%d",&data.arr[i]);
	}
	for(int i=0; i<n; i++)
	{
		printf("User :sending %d to kernel\n",data.arr[i]);
	}
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("User: got back %d %d from kernel\n",data.sum,data.avg);
	close(fd);
	return 0;
}

