#include<stdio.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#define SIZE 100
#define MAGIC_NUMBER 'B'
struct sort
{
	int arr[SIZE];
	int len;
}kernel_value;
#define SORT_INT _IOWR(MAGIC_NUMBER,1,struct sort)
int main()
{
	struct sort var;
		var.len=5;
	printf("Enter elements\n");
	for(int i=0;i<5;i++)
		scanf("%d",&var.arr[i]);
	int fd=open("/dev/sort_int",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return -1;
	}
	ioctl(fd,SORT_INT,&var);
	for(int i=0;i<5;i++)
		printf("%d ",var.arr[i]);
	printf("\n");
}
