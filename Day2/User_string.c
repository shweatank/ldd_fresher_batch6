#include<stdio.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>

int main()
{
  
  int fd=open("/dev/basic_char",O_WRONLY|O_CREAT|O_TRUNC,0666);
  if(fd<0)
  {
   perror("open");
   return 1;
  }
  printf("file sucess opened\n");
  char buf[100];
  printf("Enter a string\n");
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
  close(fd);
 
  
   fd=open("/dev/basic_char",O_RDONLY);
  if(fd<0)
  {
   perror("open");
   return 1;
  }
  printf("file sucess opened\n");
  
  n=0;
  if((n=read(fd,buf,sizeof(buf)))<0)
  {
     perror("read");
     return 1;
  }
  printf("received: %s\n",buf);
  close(fd);
 

  return 0;
}
