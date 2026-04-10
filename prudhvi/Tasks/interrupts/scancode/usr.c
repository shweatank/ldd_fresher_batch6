//minimal user peroblem calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct num)
struct num 
{
	int a ,b;
};
int main()
{
	int fd;
	fd=open("/dev/scan_code1",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	struct num v;
	printf("enter the number like a b operation add sub mul\n");
	scanf("%d%d",&v.a,&v.b);
	printf("user:sending %d %d to kernel\n",v.a,v.b);

	ioctl(fd,IOCTL_SET_VALUE,&v);
	printf("send successfully...\n");
	close(fd);
	return 0;
}
