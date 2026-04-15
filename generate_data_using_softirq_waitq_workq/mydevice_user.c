#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>


char buff[128];

int main()
{
	
	int fd;

	fd = open("/dev/mydevice", O_RDWR);

	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	char msg[] = "Generate data\n";
	write(fd, msg, sizeof( msg ));
	read(fd, buff, sizeof( buff ));

	printf("User: got back %s from kernel\n",buff);
	

	close(fd);
	return 0;
}
