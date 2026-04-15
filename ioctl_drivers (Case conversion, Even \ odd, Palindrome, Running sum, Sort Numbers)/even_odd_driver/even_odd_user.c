#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define IOCTL_MAGIC 'A'
#define EVEN_ODD_IOCTL  _IOWR(IOCTL_MAGIC, 1, int)

int main()
{
	
	int fd;
        int num;
	printf("Enter the Numer to check even or odd : ");
	scanf("%d",&num);
	

	fd = open("/dev/even_odd_driver", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	 ioctl(fd, EVEN_ODD_IOCTL, &num);
	 printf("User: got back %d from kernel\n",num);
	

	close(fd);
	return 0;
}
