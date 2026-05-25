#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>

#define IOCTL_MAGIC 'p'
#define IOCTL_STATUS _IOR(IOCTL_MAGIC,1,int)
#define IOCTL_CHANGE _IOW(IOCTL_MAGIC,2,int)

int main()
{
	int fd = open("/dev/led_gpio",O_RDWR);
	int num;

	if(fd < 0)
	{
		perror("open");
		return -1;
	}

	while(1)
	{
		printf("Choose from the below options:\n1.Check status\n2.Give option according to the status\n3.Exit\nop = ");
		int op;
		scanf("%d",&op);

		switch(op)
		{
			case 1:
				ioctl(fd, IOCTL_STATUS,&num);
				printf("Status = %d\n",num);
				break;

			case 2:
				printf("Enter the number:");
				scanf("%d",&num);
				ioctl(fd,IOCTL_CHANGE,&num);
				break;

			case 3:
				exit(0);
				break;
		}
	}

	close(fd);
	return 0;
}
