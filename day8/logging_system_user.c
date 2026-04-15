#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
int main()
{
	int fd;
	char wbuf[100];
        char rbuf[100];	
	fd=open("/dev/deferred_logger",O_RDWR);
	printf("enter the data:");
	fgets(wbuf,sizeof(wbuf),stdin);
	write(fd,wbuf,sizeof(wbuf));
	printf("Waiting for the data..\n");
	read(fd,rbuf,sizeof(rbuf));
	printf("Received :%s\n",rbuf);
	close(fd);
	return 0;
}
