#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>


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

	write( fd, &num, sizeof(int)); 
	read( fd, &num, sizeof(int)); 
	printf("User: got back %d from kernel\n",num);
	

	close(fd);
	return 0;
}
