#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/types.h>
#include<string.h>
int main()
{

	int fd=open("/dev/m_file",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("driver is opened...\n");
	char r[100],s[20]="hello india\n";


	int t=write(fd,s,strlen(s));
	if(t<0)
	{
		perror("write");
		return 1;
	}
	printf("successfully write in driver...\n");
lseek(fd,0,SEEK_SET);
	t=read(fd,r,100);
	if(t<0)
	{
		perror("read");
		return 1;
	}
	printf("successfully read in driver...\n");
	r[t]='\0';

	printf("%s\n",r);
close(fd);






}
