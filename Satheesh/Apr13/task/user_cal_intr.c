#include<stdio.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<fcntl.h>
struct two
{
	int a,b;
}v;
#define DEVICE_NAME "intr_cal"
#define MAGIC_NUMBER 'B'
#define SND_NUM _IOW(MAGIC_NUMBER,1,struct two)
int main()
{
	printf("Enter two numbers\n");
	scanf("%d%d\n",&v.a,&v.b);
	int fd=open("/dev/intr_cal",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	ioctl(fd,SND_NUM,&v);
}
