#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include <linux/minmax.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int buffer_size;

/* Open */
static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "basic_char: device opened\n");
    return 0;
}

/* Release */
static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "basic_char: device closed\n");
    return 0;
}

/* Write: perform calculation */
static ssize_t basic_write(struct file *file,
                           const char __user *user_buffer,
                           size_t count,
                           loff_t *offset)
{
    int a = 0, b = 0, result = 0;
    char op[8];
    int ret;

    if (count >= BUF_SIZE)
        count = BUF_SIZE - 1;

    if (copy_from_user(kernel_buffer, user_buffer, count))
        return -EFAULT;

    kernel_buffer[count] = '\0';

    /* Parse input */
    ret = sscanf(kernel_buffer, "%d %d %7s", &a, &b, op);
    if (ret != 3) {
        buffer_size = 0;
        return count;
    }

    /* Perform operation */
    if (strcmp(op, "add") == 0)
        result = a + b;
    else if (strcmp(op, "sub") == 0)
        result = a - b;
    else if (strcmp(op, "mul") == 0)
        result = a * b;
    else if (strcmp(op, "div") == 0) {
        if (b == 0) {
            buffer_size = 0;
            return count;
        }
        result = a / b;
    } else {
        buffer_size = 0;
        return count;
    }

    /* Store result */
    buffer_size = snprintf(kernel_buffer, BUF_SIZE, "%d\n", result);

    printk(KERN_INFO "basic_char: calculated result = %d\n", result);

    return count;
}

/* Read: send result to user */
static ssize_t basic_read(struct file *file,
                          char __user *user_buffer,
                          size_t count,
                          loff_t *offset)
{
    size_t bytes_to_copy;

    if (*offset >= buffer_size)
        return 0;

    bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

    if (copy_to_user(user_buffer,
                     kernel_buffer + *offset,
                     bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    printk(KERN_INFO "basic_char: read %zu bytes\n", bytes_to_copy);

    return bytes_to_copy;
}

/* File operations */
static struct file_operations basic_fops = {
    .owner   = THIS_MODULE,
    .open    = basic_open,
    .read    = basic_read,
    .write   = basic_write,
    .release = basic_release,
};

/* Init */
static int __init basic_char_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &basic_fops);

    if (major_number < 0) {
        printk(KERN_ALERT "basic_char: failed to register\n");
        return major_number;
    }

    printk(KERN_INFO "basic_char: loaded\n");
    printk(KERN_INFO "basic_char: major number = %d\n", major_number);
    printk(KERN_INFO "Create device using:\n");
    printk(KERN_INFO "mknod /dev/%s c %d 0\n", DEVICE_NAME, major_number);

    return 0;
}

/* Exit */
static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "basic_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("Basic Character Driver with Calculator");
