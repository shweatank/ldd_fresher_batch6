#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 1, int)
#define IOCTL_GET_VALUE _IOR(IOCTL_MAGIC, 1, char)
int main(void)
{
	int num;
	int fd;
	printf("Enter Number :");
	scanf("%d",&num);

	fd=open("/dev/even_odd_ioctl_driver",O_RDWR);
	if(fd < 0)
	{
		perror("open");
		return 1;
	}

	printf("User: Sending %d to kernel\n",num);

	ioctl(fd, IOCTL_SET_VALUE, &num);
	char str[10];
	ioctl(fd, IOCTL_GET_VALUE, str);
	printf("User: got back %s from kernel\n",str);

	close(fd);
	return 0;
}
