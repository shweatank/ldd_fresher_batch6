#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
struct two
{
	int a,b;
};
#define DEVICE_NAME "intr_cal"
#define MAGIC_NUMBER 'B'
#define SND_NUM _IOW(MAGIC_NUMBER,1,struct two)
int main()
{
	struct two v;
	printf("Enter 2 values\n");
	scanf("%d%d",&v.a,&v.b);
	int fd=open("/dev"DEVICE_NAME,O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 0;
	}	
	ioctl(fd,SND_NUM,&v);
	
	printf("sent successfully\n");
}
