//basic_ioctl_user.c
//minimal user program calling ioctl

#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/ioctl.h>
#include<stdio_ext.h>



struct ope
{
        int data1;
        int data2;
        char p;
};

int final=0;

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,int)
#define IOCTL_CAL_VALUE _IOWR(IOCTL_MAGIC,2,struct ope)
#define IOCTL_GETCAL_VALUE _IOWR(IOCTL_MAGIC,3,int)

int main()
{
        int fd;
        struct ope var;
        scanf("%d",&var.data1);
        scanf("%d",&var.data2);
        __fpurge(stdin);
        scanf("%c",&var.p);
        

      fd=open("/dev/basic_ioctl_drv3",O_RDWR);
        if(fd<0)
        {
                perror("open");
                return 1;
        }

       
        ioctl(fd,IOCTL_CAL_VALUE,&var);
        perror("ioctl");

        ioctl(fd,IOCTL_GETCAL_VALUE,&final);

        printf("user: got back %d from kernel \n",final);
        close(fd);
        return 0;
}
