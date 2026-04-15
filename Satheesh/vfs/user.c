#include<stdio.h>
#include<string.h>
#include<fcntl.h>
int main()
{
	int fd=open("/sys/kernel/sysfs_demo/value",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 0;
	}
	int num1,num2;
	printf("Enter numbers\n");
	scanf("%d%d",&num1,&num2);
	char buf[20];
	sprintf(buf,"%d %d %s",num1,num2);
	printf("in buffer %s",buf);
	char op[10];
	read(fd,op,sizeof(op));
	printf("Value: %s\n",op);
}
