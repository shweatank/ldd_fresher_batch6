#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, struct Operation)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, int)
struct Operation
{
	int num1;
	int num2;
	char opp[4];
};
struct Operation user;
int main(void)
{
	int fd;
	printf("Enter Num1 and Num2 and operation :");
	scanf("%d %d %s",&user.num1,&user.num2,user.opp);

	fd=open("/dev/calc_ioctl_driver",O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	//printf("User: Sending %d to kernel\n",&user);

	ioctl(fd, IOCTL_SET_VALUE, &user);
	int res;
	ioctl(fd, IOCTL_GET_VALUE, &res);
	printf("User: got back %d from kernel\n",res);

	close(fd);
	return 0;
}
