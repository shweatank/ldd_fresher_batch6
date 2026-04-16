#include<stdio.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>

int main()
{
	int fd=open("/proc/proc_demo",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 0;
	}
	char str[10];
	int num1,num2;
	printf("Enter operations sum sub mul div\n");
	scanf("%s",str);
	printf("Enter numbers\n");
	scanf("%d%d",&num1,&num2);
	char buf[20];
	sprintf(buf,"%d %d %s",num1,num2,str);
//	printf("in buffer %s",buf);
	write(fd,buf,strlen(buf));
	char op[20];
	lseek(fd,0,SEEK_SET);
	read(fd,op,sizeof(op));
	printf("Value: %s\n",op);
	close(fd);
}
