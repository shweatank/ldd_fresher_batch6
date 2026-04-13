#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/ioctl.h>

#define MAGIC_NUMBER 'a'
#define IOCTL_SET_MODE _IOW(MAGIC_NUMBER,1,int)
#define IOCTL_GET_MODE _IOR(MAGIC_NUMBER,2,int)
#define IOCTL_CLEAR_BUFFER  _IO(MAGIC_NUMBER,3)
#define IOCTL_GET_WCOUNT _IOR(MAGIC_NUMBER,4,int)


int main()
{
  
  int fd=open("/dev/ioctl_dev",O_RDWR);
  if(fd<0)
  {
   perror("open");
   return 1;
  }
  printf("file sucess opened\n");
  int mode=5;
  if(ioctl(fd,IOCTL_SET_MODE, &mode)<0)
	  perror("ioctl SET_MODE");
  if(ioctl(fd,IOCTL_GET_MODE, &mode)<0)
	  perror("ioctl GET_MODE");

  else
	printf("Device mode: %d\n",mode);

  char buf[100];
  printf("Enter data\n");
  fgets(buf,sizeof(buf),stdin);
  if(buf[strlen(buf)-1]=='\n')
	  buf[strlen(buf)-1]='\0';
  int n;
  if((n=write(fd,buf,sizeof(buf)))<0)
  {
     perror("write");
     return 1;
  }
  printf("send: %s\n",buf);  
  n=0;
  if((n=read(fd,buf,sizeof(buf)))<0)
  {
     perror("read");
     return 1;
  }
  buf[n]='\0';
  printf("received: %s\n",buf);
  int count;
  if(ioctl(fd,IOCTL_GET_WCOUNT,&count)<0)
		 perror("ioctl GET_COUNT");
  else
     printf("Write count:%d\n",count);

  if(ioctl(fd,IOCTL_CLEAR_BUFFER)<0)
     perror("ioctl CLEAR_BUFFER");
  else
    printf("Buffer Cleared\n");
  close(fd);
  return 0;
}
