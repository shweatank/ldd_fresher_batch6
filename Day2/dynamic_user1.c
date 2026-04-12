#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>

#define DEVICE_NAME "/dev/dynamic_buffer"
#define IOCTL_SET_SIZE _IOW('a',1,int)

int main()
{
    int fd, size = 1024;
    char buffer[256];
    char read_buffer[1024];
    int n;

    fd = open(DEVICE_NAME, O_RDWR);
    if(fd < 0)
    {
        perror("open");
        return 1;
    }
    if(ioctl(fd, IOCTL_SET_SIZE, &size) < 0)
    {
        perror("ioctl");
        return 1;
    }

    printf("Enter string: ");
    fgets(buffer, sizeof(buffer), stdin);
    buffer[strcspn(buffer, "\n")] = '\0';

    if(write(fd, buffer, strlen(buffer)) < 0)
    {
        perror("write");
        return 1;
    }

    n = read(fd, read_buffer, sizeof(read_buffer));
    if(n < 0)
    {
        perror("read");
        return 1;
    }

    read_buffer[n] = '\0';

    printf("Received from kernel: %s\n", read_buffer);

    close(fd);
    return 0;
}
