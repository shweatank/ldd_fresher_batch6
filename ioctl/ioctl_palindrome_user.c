//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,char[100])

int main(void)
{
	int fd;
	char str[100];

	fd=open("/dev/ioctl_palindrome_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the string\n");
	fgets(str,100,stdin);
	str[strlen(str)-1]='\0';
	if(ioctl(fd,IOCTL_SET_VALUE,str)<0)
	{
		perror("ioctl");
		return 1;
	}
	printf("%s\n",str);

	close(fd);
	return 0;
}
