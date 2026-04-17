//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio_ext.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct calc_data*)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,int)
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

	fd=open("/dev/ioctl_cal_get_set_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the operands\n");
	scanf("%d %d",&data.a,&data.b);
	printf("Enter the operation to be done\n");
	__fpurge(stdin);
	scanf("%c",&data.op);

	if(ioctl(fd,IOCTL_SET_VALUE,&data)<0)
		perror("ioctl set");
	if(ioctl(fd,IOCTL_GET_VALUE,&result)<0)
		perror("ioctl get");
	printf("Result=%d\n",result);

	close(fd);
	return 0;
}
