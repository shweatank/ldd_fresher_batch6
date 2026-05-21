#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>


int main()
{

	int fd=open("/dev/led_gpio",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 0;
	}

	char s[5]={0};
	while(1)
	{
		if(read(fd,s,sizeof(s))<0)
		{
			perror("read");
			return 0;
		}
		s[1]='\0';
		if(s[0]=='0')
		{
			s[0]=s[0]+1;
			write(fd,s,strlen(s));
			//{
			//	perror("1write");
			//	return 0;
		//	}
		}
		else if(s[0]=='1')
		{
			s[0]=s[0]-1;
			write(fd,s,strlen(s));
			//{
			//	perror("2write");
			//	return 0;
		//	}
		}
		sleep(1);
//s[0]=0;
//s[1]=0;
	}
close(fd);
}
