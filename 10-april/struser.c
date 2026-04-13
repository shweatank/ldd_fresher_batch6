//basic_ioctl_user.c
//minimal user program calling ioctl
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<stdlib.h>
struct buff
{
	int size;
	char * str;
};
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct buff)
int main(void)
{
        int fd;
        struct buff data;
	printf("Enter size:\n");
	scanf("%d",&data.size);
	data.str=malloc(sizeof(char)*data.size);
	scanf(" %s",data.str);
        fd=open("/dev/basic_ioctl",O_RDWR);
        if(fd<0)
        {
                perror("open");
                return 1;
        }
        printf("user: sending string to kernel\n");
        ioctl(fd,IOCTL_SET_VALUE,&data);
        printf("user: got back %s from kernel\n",data.str);
        close(fd);
        return 0;
}
