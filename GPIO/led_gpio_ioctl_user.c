#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'B'

#define SET_LED _IOW(IOCTL_MAGIC, 1, int)
#define GET_LED _IOR(IOCTL_MAGIC, 2, int)

int main()
{
	int fd;
	int status;
	fd=open("/dev/led_gpio",O_RDWR);
	if(fd < 0)
        {
                perror("open");
                return 1;
        }

	while(1)
	{
		ioctl(fd, GET_LED, status);
		if(status==1)
		{
			status=0;
		}
		else if(status==0)
		{
			status=1;
		}
		ioctl(fd, SET_LED, status);
	}
	close(fd);
	return 0;
}
