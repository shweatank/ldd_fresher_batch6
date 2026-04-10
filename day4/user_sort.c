#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
int main()
{
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	if(fd==-1)
	{
		perror("open");
		return 1;
	}
	char input[100];
	char output[100];
	printf("enter the numbers:");
	fgets(input,sizeof(input),stdin);
	write(fd,input,strlen(input));
	close(fd);
        fd=open("/dev/basic_char",O_RDONLY);
        if(fd==-1)
        {
                perror("open");
                return 1;
        }
	int n;
        if((n=read(fd,output,sizeof(output)-1))<0)
        {
                perror("reaad");
                return 1;
        }
	output[n]='\0';
	printf("Sorted :%s\n",output);
        close(fd);

}
