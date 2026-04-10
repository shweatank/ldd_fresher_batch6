//minimal user peroblem calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
struct operation 
{
	int a ,b;
	char s[10];
};
int main()
{
	int fd;
	fd=open("/dev/basic_ioctl_dev",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	struct operation v;
printf("enter the number like a b operation add sub mul\n");
scanf("%d%d%s",&v.a,&v.b,v.s);
	printf("user:sending %s %d %d to kernel\n",v.s,v.a,v.b);

	ioctl(fd,IOCTL_SET_VALUE,&v);
	printf("user:got back result %d from kernel\n",v.a);
	close(fd);
	return 0;
}
