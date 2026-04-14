#include<stdio.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
#define THIS_FILE "ioctl_practice"i
static int n;
#define MAGIC_NUMBER 'Z'
#define IOR _IOR(MAGIC_NUMBER,1,int)
#define IOW _IOW(MAGIC_NUMBER,2,int)
static int major;
int main()
{
	int k;
	printf("Enter a number\n");
	scanf("%d",&k);
	int fd=open("/dev/ioctl_practice",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return -1;
	}
	ioctl(fd,IOW,&k);
	ioctl(fd,IOR,&n);
	printf("Received %d\n",n);

}
