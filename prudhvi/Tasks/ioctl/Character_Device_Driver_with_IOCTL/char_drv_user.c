#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#define IOCTL_MAGIC 'S'
#define IOCTL_SET_DATA _IOW(IOCTL_MAGIC,1,char*)
#define IOCTL_GET_DATA _IOR(IOCTL_MAGIC,2,char*)
#define IOCTL_GET_WRDATA _IOR(IOCTL_MAGIC,3,int)
#define IOCTL_CLEAR _IO(IOCTL_MAGIC,4)
#define BUF_SIZE 256


int main()
{

	char s[BUF_SIZE],r[BUF_SIZE];
	int fd=open("/dev/char_drv",O_RDWR);
	int op;
	while(1)
	{
		printf("enter the your option...\n1)send 2)recevie 3)clear 4)get the write count\n");
		scanf("%d",&op);
		switch(op)
		{
			case 1:
				printf("enter the string....\n");
				getchar();
				scanf("%[^\n]",s);
				ioctl(fd,IOCTL_SET_DATA,s);
				printf("send DATA successfully\n");
				break;
			case 2:
				ioctl(fd,IOCTL_GET_DATA,r);
				printf("recevied DATA successfully\n");
				printf("%s\n",r);
				printf("%s\n",r);
				break;
			case 3:
				ioctl(fd,IOCTL_CLEAR);
				printf("clear DATA successfully\n");
				break;
			case 4:
				int count=0;
				ioctl(fd,IOCTL_GET_WRDATA,&count);
				printf("no of writes %d\n",count);
				break;
			default :
				printf("choose correct option...\n");
		}

	}
return 0;

}
