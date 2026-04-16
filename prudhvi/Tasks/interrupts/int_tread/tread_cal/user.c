#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>

struct data
{
	int a,b;
};
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct data)


int main()
{

	int fd=open("/dev/tread_op",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 0;
	}
	printf("enter the number...\n");
	struct data t;
	scanf("%d%d",&t.a,&t.b);
	ioctl(fd,IOCTL_SET_VALUE,&t);

	printf("sent successfully...\n");
	close(fd);


}
