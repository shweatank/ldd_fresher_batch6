#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<errno.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, struct Operation)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, int)
struct Operation
{
	int num1;
	int num2;
};
struct Operation user;
int main(void)
{
	int fd;
	printf("Enter Num1 and Num2:");
	scanf("%d %d",&user.num1,&user.num2);

	fd=open("/dev/add_using_thread_driver",O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	//printf("User: Sending %d to kernel\n",&user);

	ioctl(fd, IOCTL_SET_VALUE, &user);
	int res;
	while(ioctl(fd, IOCTL_GET_VALUE, &res)==-1)
	{
		if(errno == EAGAIN)
		{
			printf("Waiting for interrupt...\n");
			sleep(3);
		}
	}
	printf("User: got back %d from kernel\n",res);

	close(fd);
	return 0;
}
