// array_ioctl_driver.c

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>

#define DEVICE_NAME "mydevice"
#define CLASS_NAME  "myclass"

#define MY_IOCTL_MAGIC 'k'
#define ARRAY_OP _IOWR(MY_IOCTL_MAGIC, 1, struct arr_data)

// Structure
struct arr_data {
    int size;
    int arr[100];
    int sum;
    int avg;
};

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *cls;
static struct device *dev;

// IOCTL
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct arr_data data;
    int i;

    switch (cmd)
    {
        case ARRAY_OP:

            if (copy_from_user(&data, (struct arr_data *)arg, sizeof(data)))
                return -EFAULT;

            data.sum = 0;
            for (i = 0; i < data.size; i++)
                data.sum += data.arr[i];

            if (data.size > 0)
                data.avg = (int)data.sum / data.size;
            else
                data.avg = 0;

            printk(KERN_INFO "Sum=%d Avg=%d\n", data.sum, data.avg);

            if (copy_to_user((struct arr_data *)arg, &data, sizeof(data)))
                return -EFAULT;

            break;

        default:
            return -EINVAL;
    }

    return 0;
}

// File operations
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
};

// Init
static int __init my_init(void)
{
    alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    cdev_init(&my_cdev, &fops);
    cdev_add(&my_cdev, dev_num, 1);

    cls = class_create(CLASS_NAME);
    dev = device_create(cls, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "Driver loaded\n");
    return 0;
}

// Exit
static void __exit my_exit(void)
{
    device_destroy(cls, dev_num);
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Driver removed\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("IOCTL Array Driver");
