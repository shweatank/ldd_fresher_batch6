//minimal user peroblem calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#define IOCTL_MAGIC 'K'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char*)

int main()
{
	int fd;
	char s[256];
	fd =open("/dev/ioctl_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
printf("enter the string...\n");
scanf("%[^\n]",s);
printf("user:sending %s to kernel\n",s);

	ioctl(fd,IOCTL_SET_VALUE,s);
	printf("user:got back %s from kernel\n",s);
	close(fd);
	return 0;
}
