#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<stdlib.h>
int main(int argc,char * argv[])
{
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	write(fd,argv+2,2*atoi(argv[1]));
	printf("writing done successfully\n");
	close(fd);
	fd=open("/dev/basic_char",O_RDONLY);
	char buff[100];
	int n=read(fd,buff,sizeof(buff));
	buff[n]='\0';
	printf("%s\n",buff);
	//write(1,buff,strlen(buff));
	printf("read successfully\n");
	return 0;
}

