#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#define DEVICE "/dev/casechange_waitq_driver"
char write_buff[100];
char read_buff[100];
static int fd;
int main()
{
	fd=open(DEVICE,O_RDWR);
	printf("Enter the string :");
	scanf("%[^\n]",write_buff);
	if(fd==-1)
	{
		perror("File is not opened\n");
		return 0;
	}
	printf("Writing into buffer !\n");
	write(fd,&write_buff,sizeof(write_buff));
	
	lseek(fd,0,SEEK_SET);

	int n=read(fd,&read_buff,sizeof(read_buff));
	if(n>0)
	{
		read_buff[n]='\0';
		printf(" %s\n",read_buff);
	}

	close(fd);

}



