#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<string.h>
#include <stdlib.h>

int main()
{
	int fd = open("/sys/kernel/sysfs_demo/gpio",O_RDWR);

	if(fd < 0)
	{
		perror("open");
		return -1;
	}

	char buf[2];

	while(1)
	{
		printf("Choose the options:\n1.For Status\n2.For Status change\n3.Exit\nOption:");
		int op;
		scanf("%d",&op);

		switch(op)
		{
			case 1:
				int r = read(fd,buf,1);
				buf[1] = '\0';
				printf("Status = %s\n",buf);
				break;

			case 2:
				printf("Enter the status:");
				scanf("%1s",buf);
				if(buf[0] == '1' || '0')
				{
					write(fd,buf,strlen(buf));
				}
				else
				{
					printf("Enter only 0 or 1\n");
				}
				break;
			case 3:
				exit(0);
				break;
		}

	}
	return 0;
}
