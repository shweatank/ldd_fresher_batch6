#include<stdio.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#define SIZE 100
#define MAGIC_NUMBER 'B'
#define DEVICE_NAME "SUM_AVG"
#define SIZE 100
struct sort
{
	int arr[SIZE];
	int len;
}kernel_value;
#define ARR_WRITE _IOW(MAGIC_NUMBER,1,struct sort)
#define ARR_SUM _IOR(MAGIC_NUMBER,2,int) 
#define ARR_AVG _IOR(MAGIC_NUMBER,3,int)
int main()
{
	struct sort var;
		var.len=5;
	printf("Enter elements\n");
	for(int i=0;i<5;i++)
		scanf("%d",&var.arr[i]);
	int fd=open("/dev/"DEVICE_NAME,O_RDWR);
	if(fd<0)
	{
		perror("open");
		return -1;
	}
	ioctl(fd,ARR_WRITE,&var);
	int sum,average;
	ioctl(fd,ARR_SUM,&sum);
	ioctl(fd,ARR_AVG,&average);
	printf("Average: %d \nsum:%d\n",average,sum);
}
