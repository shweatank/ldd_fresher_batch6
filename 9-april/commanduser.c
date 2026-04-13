#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#define IOC_MAGIC 'C'
struct calc
{
	int a;
	int b;
	long result;
};
#define CALC_IOC_ADD _IOWR(IOC_MAGIC,1,struct calc)
#define CALC_IOC_SUB _IOWR(IOC_MAGIC,2,struct calc)
#define CALC_IOC_MUL _IOWR(IOC_MAGIC,3,struct calc)
#define CALC_IOC_DIV _IOWR(IOC_MAGIC,4,struct calc)
#define CALC_IOC_MOD _IOWR(IOC_MAGIC,5,struct calc)
static unsigned long option(const char * str)
{
	if(strcmp(str,"add")==0)
		return CALC_IOC_ADD;
	if(strcmp(str,"sub")==0)
		return CALC_IOC_SUB;
	if(strcmp(str,"mul")==0)
		return CALC_IOC_MUL;
	if(strcmp(str,"div")==0)
		return CALC_IOC_DIV;
	if(strcmp(str,"mod")==0)
		return CALC_IOC_MOD;
}
int main(int argc,char * argv[])
{
	struct calc data;
	data.a=atoi(argv[1]);
	data.b=atoi(argv[2]);
	unsigned long int cmd;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	cmd=option(argv[3]);
	printf("user: sending %d %d to kernel\n",data.a,data.b);
	ioctl(fd,cmd,&data);
	printf("user: got back %ld from kernel\n",data.result);
	close(fd);
}

