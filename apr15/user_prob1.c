#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

int main()
{
	int fd;
	char buf[100];
	if(fd<0)
	{
		perror("fd error");
		return 0;
	}
	write(fd,"generate data",13);
	read(fd,buf,sizeof(buf));
	printf("Received from buffer:%s\n",buf);
	return 0;
}

