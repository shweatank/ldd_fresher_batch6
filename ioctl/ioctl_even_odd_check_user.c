//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio_ext.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,char[10])

int main(void)
{
	int fd,result;
	char str[10];
	int num;

	fd=open("/dev/ioctl_even_odd_check_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the number\n");
	scanf("%d",&num);

	if(ioctl(fd,IOCTL_SET_VALUE,&num)<0)
		perror("ioctl set");
	if(ioctl(fd,IOCTL_GET_VALUE,&str)<0)
		perror("ioctl get");
	printf("Result=%s\n",str);

	close(fd);
	return 0;
}
