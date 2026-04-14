#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include "dynamic_ioctl.h"

#define DEVICE_NAME "dyn_buf_dev"

static int major;
static char *kernel_buffer = NULL;
static size_t buffer_size = 0;
static DEFINE_MUTEX(buffer_lock);

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    size_t new_size;
    char *new_ptr;

    if (cmd != SET_BUFFER_SIZE) return -EINVAL;
    if (get_user(new_size, (size_t __user *)arg)) return -EFAULT;

    mutex_lock(&buffer_lock);
    
    // Reallocate buffer
    new_ptr = krealloc(kernel_buffer, new_size, GFP_KERNEL);
    if (!new_ptr && new_size != 0) {
        mutex_unlock(&buffer_lock);
        return -ENOMEM;
    }

    kernel_buffer = new_ptr;
    buffer_size = new_size;
    
    mutex_unlock(&buffer_lock);
    pr_info("Buffer resized to %zu bytes\n", buffer_size);
    return 0;
}

static ssize_t dev_read(struct file *file, char __user *buf, size_t len, loff_t *ppos) {
    ssize_t ret = 0;
    mutex_lock(&buffer_lock);
    
    if (*ppos >= buffer_size) goto out;
    if (len > buffer_size - *ppos) len = buffer_size - *ppos;

    if (copy_to_user(buf, kernel_buffer + *ppos, len)) {
        ret = -EFAULT;
        goto out;
    }

    *ppos += len;
    ret = len;

out:
    mutex_unlock(&buffer_lock);
    return ret;
}

static ssize_t dev_write(struct file *file, const char __user *buf, size_t len, loff_t *ppos) {
    ssize_t ret = 0;
    mutex_lock(&buffer_lock);

    if (*ppos >= buffer_size) goto out;
    if (len > buffer_size - *ppos) len = buffer_size - *ppos;

    if (copy_from_user(kernel_buffer + *ppos, buf, len)) {
        ret = -EFAULT;
        goto out;
    }

    *ppos += len;
    ret = len;

out:
    mutex_unlock(&buffer_lock);
    return ret;
}

static struct file_operations fops = {
    .unlocked_ioctl = dev_ioctl,
    .read = dev_read,
    .write = dev_write,
};

static int __init dyn_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    return 0;
}

static void __exit dyn_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
    kfree(kernel_buffer);
}

module_init(dyn_init);
module_exit(dyn_exit);
MODULE_LICENSE("GPL");

