#include<stdio.h>
#include<fcntl.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main()
{
	int fd=open("/proc/proc_demo",O_WRONLY);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char s[30];
	printf("enter the string...\n");
//	fgets(s,"%s",stdin);
scanf("%s",s);
	if(write(fd,s,strlen(s))<0)
	{
		perror("write");
		return 1;

	}
	close(fd);
	fd=open("/proc/proc_demo",O_RDONLY);

	if(fd<0)
	{
		perror("open");
		return 1;
	}
	char r[30];
	int n;
	if((n=read(fd,r,sizeof(r)))<0)
	{
		perror("read");
		return 1;

	}
	printf("%s\n",r);


}
