#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main()
{
	int fd;
	char buf[100];
	fd=open("/dev/waitq_basic",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	while(1)
	{
		read(fd,buf,sizeof(buf));
		printf("From kernel:%s\n",buf);
	}
	close(fd);
	return 0;
}
