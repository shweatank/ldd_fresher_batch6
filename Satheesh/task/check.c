#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
int main()
{
	char ch[100];
	printf("Enter a string\n");
	scanf("%s",ch);
	int fd=open("/dev/basic_char",O_RDWR);
	int i=5;
	write(fd,&i,4);
	char temp[100];
	lseek(fd,0,SEEK_SET);
	int k;
	read(fd,&k,4);
	printf("Converted string :%d\n",k);
}
