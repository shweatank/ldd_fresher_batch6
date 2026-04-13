#define IOCTL_MAGIC 'B'
struct data
{
        int mode;
        int speed;
        char name[32];
};
#define IOCTL_SET_CONFIGURATION _IOWR(IOCTL_MAGIC,1,struct data)
#define IOCTL_GET_CONFIGURATION _IOWR(IOCTL_MAGIC,2,struct data)
#define IOCTL_RESET_CONFIGURATION _IOWR(IOCTL_MAGIC,3,struct data)
//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
int main(void)
{
	int fd;
	struct data data;
	char ch;
	fd=open("/dev/basic_ioctl",O_RDWR);
        if(fd<0)
        {
                perror("open");
                return 1;
        }
	printf("1. set_value\n2. reset_value\n");
	printf("Enter your choice\n");
	scanf(" %c",&ch);
	switch(ch)
	{
        		
		case '1': printf("Enter values to set configuration\n");
			scanf("%d %d %s",&data.mode,&data.speed,data.name);
			printf("user: sending %d %d %s to kernel\n",data.mode,data.speed,data.name);
			ioctl(fd,IOCTL_SET_CONFIGURATION,&data);
			break;
		case '2': ioctl(fd,IOCTL_RESET_CONFIGURATION,&data);
			break;
	}
	ioctl(fd,IOCTL_GET_CONFIGURATION,&data);
	printf("user: got back %d %d %s from kernel\n",data.mode,data.speed,data.name);
	close(fd);
	return 0;
}

