#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main(int argc,char *argv[])
{
	if(argc<4)
	{
		puts("Insufficient inputs");
		return 0;
	}
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	if(fd==-1)
	{
		perror("open");
		return 1;
	}
	char buf[100]={0};
	strcat(buf,argv[1]);
	strcat(buf," ");
	strcat(buf,argv[2]);
	strcat(buf," ");
	strcat(buf,argv[3]);
	strcat(buf," ");
	write(fd,buf,strlen(buf));
	close(fd);
        fd=open("/dev/basic_char",O_RDONLY);
        if(fd==-1)
        {
                perror("open");
                return 1;
        }
	char str[100];
	int n;
        if((n=read(fd,str,sizeof(str)-1))<0)
        {
                perror("reaad");
                return 1;
        }
	str[n]='\0';
	printf("The file content is :%s\n",str);
        close(fd);

}
