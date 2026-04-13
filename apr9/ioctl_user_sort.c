//basic_ioctl_user.c
//minimal user program calling ioctl

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
struct sort
{
	int n;
	int arr[100];
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct sort)

int main(int argc,char *argv[])
{
	if(argc<1)
	{
		printf("Insufficient arguments\n");
		return -1;
	}
	struct sort data;
	int fd;
	fd=open("/dev/basic_ioctl",O_RDWR);
	if(fd<0)
	{
		perror("open");
		return 1;
	}
	data.n=atoi(argv[1]);
	int i=2,j=0;
	while(argv[i]!='\0')
	{
		if(argv[i][0]>='0' && argv[i][0]<='9')
		{
			data.arr[j++]=atoi(argv[i]);
		}
		i++;
	}
	data.n=j;
	for(int k=0; k<data.n; k++)
	{
		printf("User :sending %d to kernel\n",data.arr[k]);
	}

	ioctl(fd,IOCTL_SET_VALUE,&data);
	for(int k=0; k<data.n; k++)
	{
		printf("User: got back %d from kernel\n",data.arr[k]);
	}
	close(fd);
	return 0;
}

