#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include<unistd.h>

struct device_config {
    int mode;
    int speed;
    char name[32];
};

#define MY_IOCTL_MAGIC 'k'
#define SET_CONFIG   _IOW(MY_IOCTL_MAGIC, 1, struct device_config)
#define GET_CONFIG   _IOR(MY_IOCTL_MAGIC, 2, struct device_config)
#define RESET_CONFIG _IO(MY_IOCTL_MAGIC, 3)

int main()
{
    int fd = open("/dev/mydevice", O_RDWR);
    struct device_config cfg;

    // SET
    cfg.mode = 1;
    cfg.speed = 200;
    strcpy(cfg.name, "device1");
    ioctl(fd, SET_CONFIG, &cfg);

    // GET
    ioctl(fd, GET_CONFIG, &cfg);
    printf("Mode=%d Speed=%d Name=%s\n", cfg.mode, cfg.speed, cfg.name);

    // RESET
    ioctl(fd, RESET_CONFIG);

    close(fd);
    return 0;
}
