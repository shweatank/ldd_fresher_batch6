#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
int main()
{
	int fd=open("/sys/kernel/sysfs_gpio_demo/gpio",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}

	char s[5];

	while(1)
	{

		read(fd,s,5);
		s[1]='\0';
		if(s[0]=='0')
		{
			s[0]+=1;
			write(fd,s,strlen(s));

		}
		else if(s[0]=='1')
		{
			s[0]-=1;
			write(fd,s,strlen(s));

		}
		sleep(1);
	}






	close(fd);
}

