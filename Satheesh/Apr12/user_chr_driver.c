#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
int main()
{
	char arr[100];
	printf("Enter an array\n");
	scanf("%s",arr);
	int fd=open("/dev/char_driver",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char str[20];
	write(fd,arr,strlen(arr));
	read(fd,str,sizeof(str));
	printf("Received data %s\n",str);
}
