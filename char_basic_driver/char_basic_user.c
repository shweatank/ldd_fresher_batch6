#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<stdlib.h>
int main()
{
	int fd=open("/dev/char_basic_drv",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("File is opened successfully\n");
	char buf[100];
	printf("Enter a string\n");
	fgets(buf,100,stdin);
	buf[strlen(buf)-1]='\0';
	int n;
	if((n=write(fd,buf,sizeof(buf)))<0)
	{
		perror("Write");
		exit(0);
	}
	printf("send\n");
	close(fd);

	fd=open("/dev/char_basic_drv",O_RDONLY);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	printf("FIle is opened successfully\n");
	if((n=read(fd,buf,sizeof(buf)))<0)
	{
		perror("read");
		exit(0);
	}
	printf("received %s\n",buf);
}

