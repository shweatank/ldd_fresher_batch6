//basic_ioctl_user.c
//minimal user program calling icotl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

struct device_config
{
        int mode;
        int speed;
        char name[32];
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_CONFIG _IOW(IOCTL_MAGIC,1,struct device_config)
#define IOCTL_GET_CONFIG _IOR(IOCTL_MAGIC,2,struct device_config)
#define IOCTL_RESET_CONFIG _IO(IOCTL_MAGIC,3)

int main(int argc,char *argv[])
{
	if(argc!=4)
	{
		puts("Insufficient inputs");
		return 1;
	}
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	struct device_config data;
        data.mode=atoi(argv[1]);
        data.speed=atoi(argv[2]);
        strncpy(data.name,argv[3],sizeof(data.name)-1);
        data.name[sizeof(data.name)-1]='\0';
	ioctl(fd,IOCTL_SET_CONFIG,&data);
	memset(&data,0,sizeof(data));
	ioctl(fd,IOCTL_GET_CONFIG,&data);
	printf("Result from kernel=%d %d %s\n",data.mode,data.speed,data.name);
	ioctl(fd,IOCTL_RESET_CONFIG);
	ioctl(fd,IOCTL_GET_CONFIG,&data);
	printf("After reset :Result from kernel=%d %d %s\n",data.mode,data.speed,data.name);
	close(fd);
	return 0;
}
