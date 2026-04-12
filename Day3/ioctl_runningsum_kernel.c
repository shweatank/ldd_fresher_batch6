#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "sumdev"

static int sum = 0;
static int major_number;

#define MY_IOCTL_MAGIC 'k'

struct sum_data {
    int value;
    int result;
};

#define SET_SUM _IOW(MY_IOCTL_MAGIC, 1, struct sum_data)
#define GET_SUM _IOR(MY_IOCTL_MAGIC, 2, struct sum_data)

static int sumdev_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "sumdev opened\n");
    return 0;
}

static int sumdev_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "sumdev released\n");
    return 0;
}

static long sumdev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct sum_data data;

    switch (cmd)
    {
        case SET_SUM:

            if (copy_from_user(&data, (struct sum_data *)arg, sizeof(data)))
                return -EFAULT;

            sum += data.value;

            printk(KERN_INFO "Added: %d | Running Sum: %d\n",
                   data.value, sum);
            break;

        case GET_SUM:

            data.result = sum;

            if (copy_to_user((struct sum_data *)arg, &data, sizeof(data)))
                return -EFAULT;

            break;

        default:
            return -EINVAL;
    }

    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = sumdev_open,
    .release = sumdev_release,
    .unlocked_ioctl = sumdev_ioctl,
};

static int __init sumdev_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if (major_number < 0)
        return major_number;

    printk(KERN_INFO "sumdev loaded, major = %d\n", major_number);
    return 0;
}

static void __exit sumdev_exit(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
    printk(KERN_INFO "sumdev unloaded\n");
}

module_init(sumdev_init);
module_exit(sumdev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Example");
MODULE_DESCRIPTION("Running Sum IOCTL Driver");
