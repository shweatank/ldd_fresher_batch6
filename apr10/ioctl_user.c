//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
struct device
{
	int mode;
	int speed;
	char name[100];
};
#define IOCTL_MAGIC 'D'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct device)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,struct device)
#define IOCTL_RESET_VALUE _IOWR(IOCTL_MAGIC,3,struct device)

int main(int argc,char*argv[])
{

	if(argc < 1)
	{
		printf("Insufficient arguments\n");
		return -1;
	}
	struct device data;
	int fd,flag=0;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	data.mode=atoi(argv[1]);
	data.speed=atoi(argv[2]);
	strcpy(data.name,argv[3]);

	printf("User :sending %d %d %s to kernel\n",data.mode,data.speed,data.name);

	ioctl(fd,IOCTL_SET_VALUE,&data);
	printf("User: got back %d %d %s from Kernel \n",data.mode,data.speed,data.name);

	ioctl(fd,IOCTL_GET_VALUE,&data);
	printf("User: got back %d %d %s from Kernel \n",data.mode,data.speed,data.name);

	ioctl(fd,IOCTL_RESET_VALUE,&data);
	printf("User: got back after reset %d %d %s from kernel\n",data.mode,data.speed,data.name);
	close(fd);
	return 0;
}

