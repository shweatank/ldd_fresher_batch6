#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define IOCTL_MAGIC 'C'
#define RUNNING_SUM_IOCTL  _IOWR(IOCTL_MAGIC, 1, int)

int main()
{
	
	int fd;
        int num;
	printf("Enter the Numer for running sum : ");
	scanf("%d",&num);
	

	fd = open("/dev/running_sum_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	 ioctl(fd, RUNNING_SUM_IOCTL, &num);
	 printf("User: running sum  =  %d \n",num);
	

	close(fd);
	return 0;
}
