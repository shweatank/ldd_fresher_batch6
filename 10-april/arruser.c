//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>
struct data
{
	int size;
	int * arr;
	int sum;
	int avg;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct data)
int main(void)
{
        int fd;
        struct data data;
	printf("Enter size:\n");
	scanf("%d",&data.size);
	data.arr=malloc(sizeof(int)*data.size);
	int i;
	for(i=0;i<data.size;i++)
	{
		scanf("%d",&data.arr[i]);
	}
        fd=open("/dev/basic_ioctl",O_RDWR);
        if(fd<0)
        {
                perror("open");
                return 1;
        }
        printf("user: sending array to kernel\n");
        ioctl(fd,IOCTL_SET_VALUE,&data);
        printf("user: got back sum: %d and avg: %d from kernel\n",data.sum,data.avg);
        close(fd);
        return 0;
}

