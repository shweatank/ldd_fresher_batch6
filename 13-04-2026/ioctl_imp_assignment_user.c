#include<stdio.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/ioctl.h>

#define IOCTL_MAGIC_NUM 'K'

#define SET_MODE _IOW(IOCTL_MAGIC_NUM,1,int)
#define GET_MODE _IOR(IOCTL_MAGIC_NUM,2,int)
#define CLEAR_BUFFER _IO(IOCTL_MAGIC_NUM,3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC_NUM,4,int)

int mode;
int count;
char buffer[265];
int fd;


int main()
{
        fd=open("/dev/ioctl_imp_assignment",O_RDWR);
        if(fd==-1)
        {
                perror("open");
                return 0;
        }
        printf("mode before setting=%d\n",mode);
        printf("enter the  mode\n");
        scanf("%d",&mode);
        ioctl(fd,SET_MODE,&mode);
        ioctl(fd,GET_MODE,&mode);
        printf("mode after setting=%d\n",mode);


        write(fd,"hello",5);
        write(fd,"hii",3);
        write(fd,"good morning",12);
        ioctl(fd,GET_WRITE_COUNT,&count);
        printf("The write count=%d\n",count);

        read(fd,buffer,5);
        buffer[5]='\0';
        printf("%s\n",buffer);

        read(fd,buffer,7);
        buffer[7]='\0';
        printf("%s\n",buffer);


        ioctl(fd,CLEAR_BUFFER);
}

