//basic_ioctl_user.c
//minimal user program calling icotl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#define MAX_SIZE 100
struct arr
{
        int a[MAX_SIZE];
        int size;
        int sum;
	int avg;
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct arr)

int main(void)
{
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	struct arr data;
	printf("enter the size:");
	scanf("%d",&data.size);
	printf("enter the elements:");
	for(int i=0;i<data.size;i++)
	{
		scanf("%d",&data.a[i]);
	}
	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("Result from kernel=%d %d\n",data.sum,data.avg);
	close(fd);
	return 0;
}
