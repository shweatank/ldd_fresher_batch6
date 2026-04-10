#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_CONFIG  _IOW(IOCTL_MAGIC, 1, struct Operation)
#define IOCTL_GET_CONFIG  _IOR(IOCTL_MAGIC, 2, struct Operation)
#define IOCTL_RESET       _IO(IOCTL_MAGIC, 3)


struct Operation
{
	int mode;
	int speed;
	char name[32];
};
struct Operation user;
int main(void)
{
	int fd;
	int choice;
	fd=open("/dev/config_ioctl_driver",O_RDWR);
        if(fd < 0)
        {
                perror("open");
                return 1;
        }
	printf("Select the Choice :\n1.SET_CONFIGURATION\n2.GET_CONFIGURATION\n3.RESET_CONFIGURATION\n");
	scanf("%d",&choice);
	switch(choice)
	{
		case 1:

			printf("Enter mode and speed  and name :");
			scanf("%d %d %s",&user.mode,&user.speed,user.name);
  			//printf("User: Sending %d to kernel\n",&user);
	                ioctl(fd, IOCTL_SET_CONFIG, &user);
			break;
		case 2:
			ioctl(fd, IOCTL_GET_CONFIG, &user);
			printf("User: got back mode:%d speed:%d name:%s  from kernel\n",user.mode,user.speed,user.name);
			break;
		case 3:

			ioctl(fd,IOCTL_RESET);
			break;
		default:
			printf("Enter Correct Operation\n");
			close(fd);
			return 0;
	}
	close(fd);
	return 0;
}
