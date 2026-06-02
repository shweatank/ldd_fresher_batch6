// basic_ioctl_user.c
// Minimal user program calling ioctl
 
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define SIZE 100
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC, 1, char *)

int main(void)
{
     int fd;
     char str[SIZE];
     printf("Enter the String :");
     scanf("%[^\n]",str);
     fd = open("/dev/case_ioctl_driver", O_RDWR);
     if (fd < 0) {
         perror("open");
         return 1;
     }  
     printf("User: sending %s to kernel\n", str);
     ioctl(fd, IOCTL_SET_VALUE, str);
     printf("User: got back %s from kernel\n",str);
 
     close(fd);
     return 0;
}
