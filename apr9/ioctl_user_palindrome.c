//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define IOCTL_MAGIC 'D'
#define IOCTL_SET_PALINDROME_VALUE _IOWR(IOCTL_MAGIC,1,char[100])

int main(int argc,char*argv[])
{
	char str[100];
	int fd,flag=0;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	strcpy(str,argv[1]);
	printf("User :sending %s to kernel\n",str);
	ioctl(fd,IOCTL_SET_PALINDROME_VALUE,str);
	if(str[0]=='1')
	{
		printf("User: got back from Kernel is Not Palindrome\n");
	}
	else
	{
		printf("User:got back from Kerenel is Palindrome\n");
	}
	close(fd);
	return 0;
}

