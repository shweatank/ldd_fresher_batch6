#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>

#define DEVICE_NAME "basic_char"
#define BUF_SIZE 256

static int major_number;
static char kernel_buffer[BUF_SIZE];
static char driver_msg[BUF_SIZE];
static int buffer_size;

#define MY_IOCTL_MAGIC 'k'

struct number_data {
    int num;
    char result[10];
};

#define CHECK_EVEN_ODD _IOWR(MY_IOCTL_MAGIC, 1, struct number_data)

static int basic_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "basic_char: device opened\n");
    return 0;
}

static int basic_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "basic_char: device closed\n");
    return 0;
}

static ssize_t basic_read(struct file *file, char __user *user_buffer,
                          size_t count, loff_t *offset)
{
    int bytes_to_copy;

    if (*offset >= buffer_size)
        return 0;

    bytes_to_copy = min(count, (size_t)(buffer_size - *offset));

    if (copy_to_user(user_buffer, driver_msg + *offset, bytes_to_copy))
        return -EFAULT;

    *offset += bytes_to_copy;

    printk(KERN_INFO "basic_char: read %d bytes\n", bytes_to_copy);
    return bytes_to_copy;
}

static ssize_t basic_write(struct file *file, const char __user *user_buffer,
                           size_t count, loff_t *offset)
{
    int bytes_to_copy;
    int num, ret;

    bytes_to_copy = min(count, (size_t)BUF_SIZE - 1);

    if (copy_from_user(kernel_buffer, user_buffer, bytes_to_copy))
        return -EFAULT;

    kernel_buffer[bytes_to_copy] = '\0';

    ret = kstrtoint(strim(kernel_buffer), 10, &num);
    if (ret < 0)
        return -EINVAL;

    if (num % 2 == 0)
        strscpy(driver_msg, "even", sizeof(driver_msg));
    else
        strscpy(driver_msg, "odd", sizeof(driver_msg));

    buffer_size = strlen(driver_msg);

    printk(KERN_INFO "basic_char(write): %d is %s\n", num, driver_msg);

    return bytes_to_copy;
}

static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct number_data data;

    switch (cmd)
    {
        case CHECK_EVEN_ODD:

            if (copy_from_user(&data, (struct number_data *)arg, sizeof(data)))
                return -EFAULT;

            if (data.num % 2 == 0)
                strscpy(data.result, "even", sizeof(data.result));
            else
                strscpy(data.result, "odd", sizeof(data.result));

            printk(KERN_INFO "basic_char(ioctl): %d is %s\n", data.num, data.result);

            if (copy_to_user((struct number_data *)arg, &data, sizeof(data)))
                return -EFAULT;

            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations basic_fops = {
    .owner = THIS_MODULE,
    .open = basic_open,
    .read = basic_read,
    .write = basic_write,
    .release = basic_release,
    .unlocked_ioctl = basic_ioctl,
};

static int __init basic_char_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &basic_fops);

    if (major_number < 0)
        return major_number;

    printk(KERN_INFO "basic_char: loaded\n");
    printk(KERN_INFO "major number = %d\n", major_number);

    return 0;
}

static void __exit basic_char_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "basic_char: unloaded\n");
}

module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("Basic Char Driver with IOCTL");
