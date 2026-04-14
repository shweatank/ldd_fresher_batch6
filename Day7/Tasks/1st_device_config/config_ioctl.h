#ifndef CONFIG_IOCTL_H
#define CONFIG_IOCTL_H
#include <linux/ioctl.h>

struct device_config {
    int mode;
    int speed;
    char name[32];
};

#define IOCTL_MAGIC 'C'
#define SET_CONFIG   _IOW(IOCTL_MAGIC, 1, struct device_config)
#define GET_CONFIG   _IOR(IOCTL_MAGIC, 2, struct device_config)
#define RESET_CONFIG _IO(IOCTL_MAGIC, 3)

#endif

