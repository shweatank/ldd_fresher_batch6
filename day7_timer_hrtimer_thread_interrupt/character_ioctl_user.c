/* user program to send mode and commands through ioctl*/
#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>
#include<linux/ioctl.h>

#define IOCTL_MAGIC 'E'
#define SET_MODE _IOW(IOCTL_MAGIC,1,int)
#define GET_MODE _IOR(IOCTL_MAGIC,2,int)
#define CLEAR_BUFFER _IO(IOCTL_MAGIC,3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC,4,int)


int main(void){
int fd,mode,count;
char buf[256]="Hello Driver";

fd=open("/dev/my_device",O_RDWR);
if(fd<0){
perror("open");
return -1;
}

write(fd,buf,sizeof(buf));

mode=10;
ioctl(fd,SET_MODE,&mode);//set the mode
ioctl(fd,GET_MODE,mode); //get the mode

printf("Mode: %d\n",mode);

ioctl(fd,GET_WRITE_COUNT,&count);//get write count

printf("Write count: %d\n",count);

ioctl(fd,CLEAR_BUFFER);//Clear buffer

close(fd);
}

