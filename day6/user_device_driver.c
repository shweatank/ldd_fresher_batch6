#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<fcntl.h>
#include<sys/ioctl.h>

#define BUF_SIZE 1024
#define IOCTL_MAGIC 'A'
#define IOCTL_SET_MODE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_MODE _IOR(IOCTL_MAGIC,2,int)
#define IOCTL_CLEAR_BUFFER _IO(IOCTL_MAGIC,3)
#define IOCTL_GET_WCOUNT  _IOR(IOCTL_MAGIC,4,int)

int main()
{
	int fd;
	char buf[BUF_SIZE];
	fd=open("/dev/my_ioctl_dev",O_RDWR);
	if(fd==-1)
	{
		perror("open");
		return 1;
	}
	int mode=5;
	if(ioctl(fd,IOCTL_SET_MODE,&mode)<0)
	{
		perror("ioctl SET_MODE");
	}
	if(ioctl(fd,IOCTL_GET_MODE,&mode)<0)
        {
                perror("ioctl GET_MODE");
        }
	else
	{
		printf("Device mode:%d\n",mode);
	}
	printf("enter the data:");
	if(fgets(buf,sizeof(buf),stdin)!=NULL)
	{
		if(buf[strlen(buf)-1]=='\n')
			buf[strlen(buf)-1]='\0';
		if(write(fd,buf,strlen(buf))<0)
                {
                       perror("write");
                       return 1;
                }
	}
	char str[100];
	int n;
        if((n=read(fd,str,sizeof(str)-1))<0)
        {
                perror("reaad");
                return 1;
        }
	str[n]='\0';
	printf("Buffer content is :%s\n",str);
	int count;
	if(ioctl(fd,IOCTL_GET_WCOUNT,&count)<0)
		perror("ioctl GET_WCOUNT");
	else
		printf("Write count:%d\n",count);
	if(ioctl(fd,IOCTL_CLEAR_BUFFER)<0)
		perror("ioctl CLEAR_BUFFER");
	else
		printf("Buffer cleared\n");
        close(fd);

}
