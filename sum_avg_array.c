// basic_ioctl_user.c
// Minimal user program calling ioctl
 
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
 
#define IOCTL_MAGIC 'B'
#define IOCTL_SEND_ARRAY _IOW(IOCTL_MAGIC, 1, int)
#define IOCTL_RECEIVE    _IOR(IOCTL_MAGIC, 2, int)



int main(void)
{
     int fd;
     int arr[100];
     printf("Enter Array Elements :");
     for(int i=0;i<100;i++)
     {
	     scanf("%d",&arr[i]);
     }
     fd = open("/dev/sum_avg_array_driver", O_RDWR);
     if (fd < 0) {
         perror("open");
         return 1;
     }  
     printf("User: sending %d to kernel\n",arr);
     ioctl(fd, IOCTL_SET_VALUE, arr);
     printf("User: got back %d from kernel\n", value);

 
     close(fd);
     return 0;
}
