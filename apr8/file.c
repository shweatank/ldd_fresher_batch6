#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
int main(int argc,char *argv[])
{
	int fd;
	fd=open("/dev/basic_char",O_WRONLY);
	char buff[10];
	int i=1,j=0;
	while(i<argc)
	{
		int k=0;
		while(argv[i][k] !='\0')
		{
			buff[j]=argv[i][k];
			k++;
			j++;
		}
		buff[j]=' ';
		j++;
		i++;
	}
	buff[j-1]='\0';
	write(fd,buff,strlen(buff));
	printf("Writing in device driver successfully\n");
	close(fd);

	fd=open("/dev/basic_char",O_RDONLY);
	char read_buff[10];
	int n=read(fd,read_buff,sizeof(read_buff));
	read_buff[n]='\0';
	printf("Reading in device driver Successfully:%s\n",read_buff);
	close(fd);
	return 0;
}

