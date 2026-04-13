//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>
struct data
{
	int size;
	int * arr;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct data)
int main(void)
{
	struct data data;
	scanf("%d",&data.size);
	int i;
	data.arr=malloc(sizeof(int)*data.size);
	for(i=0;i<data.size;i++)
	{
		scanf("%d",&data.arr[i]);
	}
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("user: sending array to kernel\n");
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("user: got back arr from kernel\n");
	for(i=0;i<data.size;i++)
	{
		printf("%d ",data.arr[i]);
	}
	printf("\n");
	close(fd);
	return 0;
}

