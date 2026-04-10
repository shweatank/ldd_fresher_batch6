// basic_ioctl_user.c
// Minimal user program calling ioctl
 
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
 
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC, 1, int)

int main(void)
{
     int fd;
     int value = 20;
 
     fd = open("/dev/ioctl_example", O_RDWR);
     if (fd < 0) {
         perror("open");
         return 1;
     }  
     printf("User: sending %d to kernel\n", value);
     ioctl(fd, IOCTL_SET_VALUE, &value);
     printf("User: got back %d from kernel\n", value);
 
     close(fd);
     return 0;
}
