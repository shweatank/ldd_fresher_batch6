#include <stdio.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

#define IOCTL_MAGIC 'G'
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC,1,int)

int main()
{

int num1;

int fd = open("/dev/even_odd", O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }
printf("Enter the number: ");
scanf("%d",&num1);
ioctl(fd,IOCTL_SET_VALUE, &num1);
return 0;
}

