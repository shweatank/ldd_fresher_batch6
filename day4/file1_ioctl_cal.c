//basic_ioctl_user.c
//minimal user program calling icotl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>

struct calculator
{
	int num1;
	int num2;
	char op;
	int result;
};
#define IOCTL_MAGIC 'D'
#define CAL_IOCTL_ADD _IOWR(IOCTL_MAGIC,1,struct calculator)
#define CAL_IOCTL_SUB _IOWR(IOCTL_MAGIC,2,struct calculator)
#define CAL_IOCTL_MUL _IOWR(IOCTL_MAGIC,3,struct calculator)
#define CAL_IOCTL_DIV _IOWR(IOCTL_MAGIC,4,struct calculator)
#define CAL_IOCTL_MOD _IOWR(IOCTL_MAGIC,5,struct calculator)

int main(int argc,char *argv[])
{
	int fd;
	struct calculator data;
	int cmd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	data.num1=atoi(argv[1]);
	data.num2=atoi(argv[2]);
	data.op=argv[3][0];
	switch(data.op)
	{
		case '+':cmd=CAL_IOCTL_ADD;
			 break;
		case '-':cmd=CAL_IOCTL_SUB;
			 break;
		case 'X':cmd=CAL_IOCTL_MUL;
			 break;
		case '/':cmd=CAL_IOCTL_DIV;
			 break;
		case '%':cmd=CAL_IOCTL_MOD;
			 break;
		default:printf("Invalid operation\n");
			return 1;
	}
	printf("User:sending to %d %d and %c kernel\n",data.num1,data.num2,data.op);
	ioctl(fd,cmd,&data);
	printf("Result from kernel=%d\n",data.result);
	close(fd);
	return 0;
}
