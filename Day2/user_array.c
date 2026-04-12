#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>


int main()
{
  int fd;
  fd=open("/dev/basic_char",O_WRONLY);
  if(fd<0)
  {
	perror("open");
	return 1;
  }
  printf("file successfully opened\n");
  char buf[100];
  fgets(buf,sizeof(buf),stdin);
  if(buf[strlen(buf)-1]=='\n')
	  buf[strlen(buf)-1]='\0';
   int n;
   if((n=write(fd,buf,strlen(buf)))<0)
   {
	  perror("write");
	  return 1;
   }
   printf("send: %s\n",buf);

   n=0;
   fd=open("/dev/basic_char",O_RDONLY);
   if(fd<0)
   {
	  perror("open");
	  return 1;
   }
   printf("file successfully opened\n");
   if((n=read(fd,buf,sizeof(buf)))<0)
   {
          perror("read");
          return 1;
   }
   buf[n]='\0';
   for(int i=0;buf[i]!='\0';i++)
   {
   printf("received: %d ",buf[i]-'0');
   }

 return 0;
}
