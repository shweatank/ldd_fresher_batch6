#ifndef ARRAY_IOCTL_H
#define ARRAY_IOCTL_H
#include <linux/ioctl.h>

struct array_payload {
    int __user *user_ptr; // Pointer to user's array
    int size;             // Number of elements
    long sum;             // Result 1
    int avg;              // Result 2
};

#define IOCTL_MAGIC 'A'
#define PROCESS_ARRAY _IOWR(IOCTL_MAGIC, 1, struct array_payload)

#endif

