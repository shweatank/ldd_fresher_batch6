//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio_ext.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_CALC_VALUE _IOWR(IOCTL_MAGIC,1,struct calc_data*)
struct calc_data
{
	int a;
	int b;
	char op;
};

int main(void)
{
	int fd,result;
	struct calc_data data;

	fd=open("/dev/ioctl_cal_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the operands\n");
	scanf("%d %d",&data.a,&data.b);
	printf("Enter the operation to be done\n");
	__fpurge(stdin);
	data.op=getchar();

	result=ioctl(fd,IOCTL_CALC_VALUE,&data);
	printf("Result=%d\n",result);

	close(fd);
	return 0;
}
