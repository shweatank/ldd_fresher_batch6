#ifndef DYNAMIC_IOCTL_H
#define DYNAMIC_IOCTL_H
#include <linux/ioctl.h>

// Command to resize the kernel buffer
#define SET_BUFFER_SIZE _IOW('D', 1, size_t)

#endif

