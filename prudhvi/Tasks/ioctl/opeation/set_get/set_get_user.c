//minimal user peroblem calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
struct operation 
{
	int a ,b;
	char s[10];
};
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_VALUE _IOR(IOCTL_MAGIC,1,struct operation)
#define IOCTL_GET_VALUE _IOW(IOCTL_MAGIC,2,int)

int main()
{
	int fd;
int res=0;
	fd=open("/dev/set_get",O_RDWR);
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
ioctl(fd,IOCTL_GET_VALUE,&res);
	printf("user:got back result %d from kernel\n",res);
	close(fd);
	return 0;
}
