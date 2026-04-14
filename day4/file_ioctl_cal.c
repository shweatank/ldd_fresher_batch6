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
	int result;
};
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct calculator)

int main(void)
{
	int fd;
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
	printf("Result from kernel=%d\n",data.result);
	close(fd);
	return 0;
}
