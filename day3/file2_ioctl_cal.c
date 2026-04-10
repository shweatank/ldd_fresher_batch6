//basic_ioctl_user.c
//minimal user program calling icotl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

struct calculator
{
	int num1;
	int num2;
	char op;
};
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct calculator)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)

int main(void)
{
	int fd,result;
	struct calculator data;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("enter the two numbers:");
	scanf("%d %d",&data.num1,&data.num2);
	printf("enter the operator:");
	scanf(" %c",&data.op);
	ioctl(fd,IOCTL_SET_VALUE,&data);
	ioctl(fd,IOCTL_GET_VALUE,&result);
	printf("Result from kernel=%d\n",result);
	close(fd);
	return 0;
}
