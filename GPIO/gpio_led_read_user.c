#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#define DEVICE "/dev/led_gpio_read"

int main() {
    int fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }
    char msg[2]={0};
    while(1)
    {
       read(fd,msg,1);
       if(msg[0]=='0')
       {
	       write(fd,"1",1);
	       sleep(1);
       }
       else if(msg[0]=='1')
       {
	       write(fd,"0",1);
	       sleep(1);
       }
    }
    close(fd);
    return 0;
}
