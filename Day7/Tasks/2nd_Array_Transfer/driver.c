#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h> // For kmalloc/kfree
#include "array_ioctl.h"

#define DEVICE_NAME "array_dev"
static int major;

static long array_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct array_payload data;
    int *kernel_array;
    int i;

    if (cmd != PROCESS_ARRAY) return -EINVAL;

    // 1. Copy the control struct
    if (copy_from_user(&data, (struct array_payload __user *)arg, sizeof(data)))
        return -EFAULT;

    if (data.size <= 0 || data.size > 1024) return -EINVAL; // Safety check

    // 2. Allocate memory for the actual array
    kernel_array = kmalloc(sizeof(int) * data.size, GFP_KERNEL);
    if (!kernel_array) return -ENOMEM;

    // 3. Copy the actual array data
    if (copy_from_user(kernel_array, data.user_ptr, sizeof(int) * data.size)) {
        kfree(kernel_array);
        return -EFAULT;
    }

    // 4. Calculate Logic
    data.sum = 0;
    for (i = 0; i < data.size; i++) {
        data.sum += kernel_array[i];
    }
    data.avg = data.sum / data.size;

    // 5. Copy results back to the control struct in user-space
    if (copy_to_user((struct array_payload __user *)arg, &data, sizeof(data))) {
        kfree(kernel_array);
        return -EFAULT;
    }

    kfree(kernel_array);
    return 0;
}

static struct file_operations fops = { .unlocked_ioctl = array_ioctl };

static int __init arr_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("Array Driver: mknod /dev/%s c %d 0\n", DEVICE_NAME, major);
    return 0;
}

static void __exit arr_exit(void) { unregister_chrdev(major, DEVICE_NAME); }

module_init(arr_init);
module_exit(arr_exit);
MODULE_LICENSE("GPL");

