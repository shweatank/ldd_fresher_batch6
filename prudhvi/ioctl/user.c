#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#define IOCTL_MAGIC 'B'
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,1,int)
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,2,int)

int main()
{
	int fd=open("/dev/ioctl_led_gpio",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 0;
	}
	int r;
	while(1)
	{
		if(ioctl(fd,IOCTL_GET_VALUE,&r)<0)
		{
			perror("ioctl_read");
			return 0;
		}

		if(r==0)
		{
			int i=1;
			if(ioctl(fd,IOCTL_SET_VALUE,&i)<0)
			{
				perror("ioctl_write1");
				return 0;
			}

		}
		else
		{
			int i=0;
			if(ioctl(fd,IOCTL_SET_VALUE,&i)<0)
			{
				perror("ioctl_write0");
				return 0;
			}
		}
		sleep(1);
	}
	close(fd);

}

