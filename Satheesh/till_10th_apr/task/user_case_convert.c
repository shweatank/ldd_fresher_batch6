#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#define MAGIC_NUMBER 'B'
#define DEVICE_NAME "case_convert"
#define SIZE 100
#define CASE_CONVERT _IOWR(MAGIC_NUMBER,1,char *)
int main()
{
	int fd=open("/dev/case_convert",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return -1;
	}
	char ch[100];
	printf("Enter string\n");
	scanf("%[^\n]",ch);
	ioctl(fd,CASE_CONVERT,ch);
	printf("String: %s\n",ch);
}
