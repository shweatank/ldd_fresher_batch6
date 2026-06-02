#include<stdio.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define IOCTL_MAGIC 'B'
#define IOCTL_SET_MODE  _IOW(IOCTL_MAGIC, 1, int)
#define IOCTL_GET_MODE  _IOR(IOCTL_MAGIC, 2, int)
#define CLEAR_BUFFER       _IO(IOCTL_MAGIC, 3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC,4,int)


int main(void)
{
	int fd;
	int device_mode,write_count;
	char buf[256];
	fd=open("/dev/control_ioctl_driver",O_RDWR);
        if(fd < 0)
        {
                perror("open");
                return 1;
        }
	device_mode=5;
	ioctl(fd,IOCTL_SET_MODE,&device_mode);

	ioctl(fd,IOCTL_GET_MODE,&device_mode);
	printf("Device mode is %d\n",device_mode);

	write(fd,"Hello World",12);

	read(fd,&buf,sizeof(buf));
	printf("The Buffer is %s\n",buf);

	ioctl(fd,GET_WRITE_COUNT,&write_count);
	printf("The Count of Write is %d",write_count);

	ioctl(fd,CLEAR_BUFFER);

	close(fd);
	return 0;
}
