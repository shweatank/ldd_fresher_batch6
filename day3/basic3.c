#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#define BUF_SIZE 1024
int main()
{
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	if(fd==-1)
	{
		perror("open");
		return 1;
	}
	char buf[BUF_SIZE];
	if(fgets(buf,sizeof(buf),stdin)!=NULL)
	{
		if(buf[strlen(buf)-1]=='\n')
			buf[strlen(buf)-1]='\0';
		if(write(fd,buf,strlen(buf))<0)
                {
                       perror("write");
                       return 1;
                }
	}
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
