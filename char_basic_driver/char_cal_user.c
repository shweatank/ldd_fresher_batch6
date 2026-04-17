#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
int main()
{
	int fd=open("/dev/char_cal_drv",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		exit(0);
	}
	char str[100];
	printf("Enter the data\n");
	fgets(str,100,stdin);
	str[strlen(str)-1]='\0';
	int n;
	if((n=write(fd,str,strlen(str))<0))
	{
		perror("write");
		exit(0);
	}
	close(fd);

	char ptr[100];
	fd=open("/dev/char_cal_drv",O_RDONLY);
	read(fd,ptr,sizeof(ptr));
	printf("The result is %s\n",ptr);



}
