//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct op_data)
struct op_data
{
	int arr[100];
	int size;
};

int main(void)
{
	int fd;
	struct op_data data;

	fd=open("/dev/ioctl_sort_arr_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the number of elements\n");
	scanf("%d",&data.size);
	printf("Enter the elements\n");
	for(int i=0;i<data.size;i++)
	{
		scanf("%d",&data.arr[i]);
	}
	if(ioctl(fd,IOCTL_SET_VALUE,&data)<0)
	{
		perror("ioctl");
		return 1;
	}
	printf("The elements are:\n");
	for(int i=0;i<data.size;i++)
	{
		printf("%d ",data.arr[i]);
	}

	close(fd);
	return 0;
}
