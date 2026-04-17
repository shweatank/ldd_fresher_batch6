#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
	int fd=open("/sys/kernel/sysfs_demo/value",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	int s;
	printf("enter the number....\n");
char buf[20];
scanf("%d",&s);	
snprintf(buf,sizeof(buf),"%d",s);
char r[20];

	if(write(fd,buf,strlen(buf))<0)
	{
		perror("write");
		return 1;
	}
	close(fd);
	fd=open("/sys/kernel/sysfs_demo/value",O_RDONLY);
int n;
	if((n=read(fd,r,sizeof(r)))<0)
	{
		perror("read");
		return 1;
	}
r[n]='\0';
	printf("%s",r);
	close(fd);
}
