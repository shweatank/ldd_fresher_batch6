#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>

int main()
{
   char buf[100];
   int fd;
   fd=open("/proc/proc_demo",O_RDWR);
   if(fd<0)
   {
	  perror("open");
	  return -1;
   }
   
   printf("Enter a string\n");
   fgets(buf,sizeof(buf),stdin);
   if(buf[strlen(buf)-1]=='\n')
  	   buf[strlen(buf)-1]='\0';
   write(fd,buf,strlen(buf));

   int n;
  if((n=read(fd,buf,sizeof(buf)))>0)
  {
    buf[n]='\0';
    printf("received : %s\n",buf);
  } 
  close(fd);
  return 0;
}
