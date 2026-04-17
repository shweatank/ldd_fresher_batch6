//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio_ext.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct set)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,struct get)

struct set
{
	int arr[100];
	int size;
};
struct get
{
	int sum;
	int avg;
};

int main(void)
{
	int fd;
	struct set s;
	struct get g;

	fd=open("/dev/ioctl_arr_cal_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the number of elements\n");
	scanf("%d",&s.size);
	printf("Enter the elements\n");
	for(int i=0;i<s.size;i++)
	{
		scanf("%d",&s.arr[i]);
	}

	if(ioctl(fd,IOCTL_SET_VALUE,&s)<0)
		perror("ioctl set");
	if(ioctl(fd,IOCTL_GET_VALUE,&g)<0)
		perror("ioctl get");
	printf("sum=%d  avg=%d\n",g.sum,g.avg);

	close(fd);
	return 0;
}
