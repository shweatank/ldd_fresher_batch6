#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>



#define DEVICE "/dev/led_gpio_ioctl"
#define MAGIC_NUMBER 'B'
#define SET_PIN_STATUS _IOW(MAGIC_NUMBER,1,int)
#define GET_PIN_STATUS _IOR(MAGIC_NUMBER,2,int)


int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    char msg[2]={0};
    int status=0;
    while(1)
    {
       ioctl(fd,GET_PIN_STATUS,&status);
       printf("%d\n",status);
       if(status==0)
       {
	       status=1;
	       ioctl(fd,SET_PIN_STATUS,&status);
	       sleep(1);
       }
       else if(status==1)
       {
	       status=0;
	       ioctl(fd,SET_PIN_STATUS,&status);
	       sleep(1);
       }
    }
    close(fd);
    return 0;
}
