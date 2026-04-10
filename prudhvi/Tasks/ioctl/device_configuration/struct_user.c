//minimal user peroblem calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
struct config
   {
  int mode;
  int speed;
  char name[32];
  };
 
  #define IOCTL_MAGIC 'B'
  #define IOCTL_SET_CONFIG _IOW(IOCTL_MAGIC,1,struct config)
  #define IOCTL_GET_CONFIG _IOR(IOCTL_MAGIC,2,struct config)
  #define IOCTL_RESET _IO(IOCTL_MAGIC,3)
int main()
{
	int fd=open("/dev/struct_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	struct config s,r;
	printf("enter the mode speed and name...\n");
	scanf("%d%d%s",&s.mode,&s.speed,s.name);
	printf("user:sending %d %d %s to kernel\n",s.mode,s.speed,s.name);

	ioctl(fd,IOCTL_SET_CONFIG,&s);
	ioctl(fd,IOCTL_GET_CONFIG,&r);
	printf("user:receving %d %d %s to kernel\n",r.mode,r.speed,r.name);
	ioctl(fd,IOCTL_RESET);

	printf("Reset completed..\n");
	close(fd);
	return 0;
}
