#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
void main(int argc,char **argv)
{

	if(argc<4)
	{
		printf("usage:./a.out 2 3\n");
		return;
	}
	int fd=open("/dev/op",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return;
	}
	char s[100];
	sprintf(s,"%s %s %s",argv[1],argv[2],argv[3]);
	if(write(fd,s,strlen(s))<0)
	{
		perror("write");
		return;
	}
	int i;
	if((i=read(fd,s,100))<0)
	{
		perror("read");
		return;
	}
	s[i]='\0';


	printf("%s",s);


}
