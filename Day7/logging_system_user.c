#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

int main()
{
  int fd;
  fd=open("/dev/deferred_logger",O_RDWR);
  if(fd<0)
  {
    perror("open");
    return 1;
  }
  char buf[100];
  printf("Enter the data: \n");
  fgets(buf,sizeof(buf),stdin);
  if(buf[strlen(buf)-1]=='\n')
	  buf[strlen(buf)-1]='\0';
  write(fd,buf,strlen(buf));

  int n;
  if((n=read(fd,buf,strlen(buf)))>0)
  {
	 buf[n]='\0';
	 printf("Received: %s\n",buf);
  }
  close(fd);
}
