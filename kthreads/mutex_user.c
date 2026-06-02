// user_app.c

#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define DEVICE "/dev/my_mutex_dev"
#define SET_CONFIG _IOW('a', 'a', int)

int main()
{
    int fd;
    int value;

    fd = open(DEVICE, O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    printf("Enter value to set config: ");
    scanf("%d", &value);

    if (ioctl(fd, SET_CONFIG, &value) < 0) {
        perror("IOCTL failed");
        close(fd);
        return -1;
    }

    printf("Config updated successfully\n");

    close(fd);
    return 0;
}
