//basic_ioctl_user.c
//Minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdio_ext.h>
#include<string.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,struct dev_config)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC,2,struct dev_config)
#define IOCTL_RESET_VALUE _IO(IOCTL_MAGIC,3)

struct dev_config
{
	int mode;
	int speed;
	char name[32];
};

int main(void)
{
	int fd;
	struct dev_config data;

	fd=open("/dev/ioctl_dev_config_drv",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	printf("Enter the mode\n");
	scanf("%d",&data.mode);
	printf("Enter the speed\n");
	scanf("%d",&data.speed);
	printf("Enter the name\n");
	fgets(data.name,32,stdin);
	data.name[strlen(data.name)-1]='\0';
	if(ioctl(fd,IOCTL_SET_VALUE,&data)<0)
		perror("ioctl set");

	if(ioctl(fd,IOCTL_GET_VALUE,&data)<0)
		perror("ioctl get");
	printf("The config of the device is : Mode=%d speed=%d name=%s\n",data.mode,data.speed,data.name);

	if(ioctl(fd,IOCTL_RESET_VALUE)<0)
		perror("ioctl reset");

	if(ioctl(fd,IOCTL_GET_VALUE,&data)<0)
		perror("ioctl get");

	printf("The config of the device is : Mode=%d speed=%d name=%s\n",data.mode,data.speed,data.name);




	close(fd);
	return 0;
}
