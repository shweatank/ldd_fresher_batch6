#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>

int main()
{
	int val=0;
	char str[50];
	printf("enter the val1 and val2 and operation in string format\n");
	scanf(" %s",str);
	int fd=open("/dev/cal_thread".O_RDWR);

	write(fd,str,sizeof(str));
	read(fd,&val,4);
	printf("result= %d\n",val);
}

