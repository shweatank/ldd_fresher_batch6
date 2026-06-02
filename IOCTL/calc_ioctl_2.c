#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<errno.h>

#define CALC_IOC_MAGIC 'B'
//#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC, 1, struct Operation)

struct calc_req
{
	int a;
	int b;
	long result;
	int err;
};

#define CALC_IOC_ADD _IOWR(CALC_IOC_MAGIC, 1, struct calc_req)
#define CALC_IOC_SUB _IOWR(CALC_IOC_MAGIC, 2, struct calc_req)
#define CALC_IOC_MUL _IOWR(CALC_IOC_MAGIC, 3, struct calc_req)
#define CALC_IOC_DIV _IOWR(CALC_IOC_MAGIC, 4, struct calc_req)
#define CALC_IOC_MOD _IOWR(CALC_IOC_MAGIC, 5, struct calc_req)

static void usage(const char *p)
{
	fprintf(stderr,
		"Usage:\n"
	       "	%s <add|sub|mul|div|mod> <a> <b>\n"
	       "Example:\n"
	       " %s add 10 20\n",p,p);
}

static unsigned long op_to_ioctl(const char *op)
{
	if(!strcmp(op,"add")) return CALC_IOC_ADD;
	if(!strcmp(op,"sub")) return CALC_IOC_SUB;
	if(!strcmp(op,"mul")) return CALC_IOC_MUL;
	if(!strcmp(op,"div")) return CALC_IOC_DIV;
	if(!strcmp(op,"mod")) return CALC_IOC_MOD;
	return 0;
}
int main(int argc,char **argv)
{
	int fd;
	struct calc_req r;
	unsigned long cmd;
	if (argc !=4)
	{
		usage(argv[0]);
		return 1;
	}

	cmd= op_to_ioctl(argv[1]);
	if(!cmd)
	{
		fprintf(stderr, "Unknown op: %s\n",argv[1]);
		usage(argv[0]);
		return 1;
	}
	r.a=atoi(argv[2]);
	r.b=atoi(argv[3]);
	r.result=0;
	r.err=0;

	fd=open("/dev/calc_ioctl_driver2",O_RDWR)
	if(fd<0)
	{
		perror("open(/dev/calc_ioctl_driver2)");
		return 1;
	}

	if(ioctl(fd, cmd , &r)< 0)
	{
		perror("ioctl");
		close(fd);
	}
	return 0;
}
