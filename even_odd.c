#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#define DEVICE "/dev/even_odd_driver"
//char write_buff[100];
int number;
char read_buff[100];
static int fd;
int main()
{
	FILE* fd=open(DEVICE,"rw");
	if(fd==-1)
	{
		perror("File is not opened\n");
		return 0;
	}
	printf("Enter the number :");
        scanf("%d",&number);
	printf("Writing into buffer !\n");
	fprintf(fd,"%d",number);
	
	lseek(fd,0,SEEK_SET);

	int n=read(fd,read_buff,sizeof(read_buff));
	if(n>0)
	{
		read_buff[n]='\0';
		printf(" %s\n",read_buff);
	}

	close(fd);

}



