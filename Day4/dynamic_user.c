#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<sys/ioctl.h>
#include<unistd.h>

#define DEVICE_NAME "/dev/dynamic_buffer"
#define IOCTL_SET_SIZE _IOW('a',1,int)

int main()
{
   int fd, size = 1024;
   char *buffer = "Hello, world!";

   fd = open(DEVICE_NAME, O_RDWR);
   if(fd<0){
      perror("open");
      return 1;
   }

   if(ioctl(fd, IOCTL_SET_SIZE, &size))
   {
     perror("ioctl");
     return 1;
   }

   if(write(fd,buffer, 12)<0)
   {
	   perror("write");
	   return 1;
   }
   char read_buffer[1024];
   if(read(fd, read_buffer, 13)<0)
   {
	 perror("read");
	 return 1;
   }

   printf("%s\n", read_buffer);
   close(fd);
   return 0;
}
