#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/string.h>

#define DEVICE_NAME "mydevice"
#define CLASS_NAME  "myclass"

#define MY_IOCTL_MAGIC 'k'

#define SET_CONFIG   _IOW(MY_IOCTL_MAGIC, 1, struct device_config)
#define GET_CONFIG   _IOR(MY_IOCTL_MAGIC, 2, struct device_config)
#define RESET_CONFIG _IO(MY_IOCTL_MAGIC, 3)

// Device configuration structure
struct device_config {
    int mode;
    int speed;
    char name[32];
};

// Global config (device state)
static struct device_config dev_cfg = {
    .mode = 0,
    .speed = 100,
    .name = "default"
};

static dev_t dev_num;
static struct cdev my_cdev;
static struct class *cls;
static struct device *dev;

// ---------------- FILE OPERATIONS ----------------

static int my_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device opened\n");
    return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "Device closed\n");
    return 0;
}

// ---------------- IOCTL ----------------

static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct device_config temp;

    switch (cmd)
    {
        case SET_CONFIG:
            if (copy_from_user(&temp, (struct device_config *)arg, sizeof(temp)))
                return -EFAULT;

            dev_cfg = temp;
            printk(KERN_INFO "SET: mode=%d speed=%d name=%s\n",
                   dev_cfg.mode, dev_cfg.speed, dev_cfg.name);
            break;

        case GET_CONFIG:
            if (copy_to_user((struct device_config *)arg, &dev_cfg, sizeof(dev_cfg)))
                return -EFAULT;
            break;

        case RESET_CONFIG:
            dev_cfg.mode = 0;
            dev_cfg.speed = 100;
            strcpy(dev_cfg.name, "default");
            printk(KERN_INFO "RESET to default\n");
            break;

        default:
            return -EINVAL;
    }

    return 0;
}

// ---------------- FOPS ----------------

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = my_open,
    .release = my_release,
    .unlocked_ioctl = my_ioctl,
};

// ---------------- INIT ----------------

static int __init my_init(void)
{
    // Allocate device number
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
    {
        printk(KERN_ERR "Device number allocation failed\n");
        return -1;
    }

    // Initialize cdev
    cdev_init(&my_cdev, &fops);

    // Add cdev
    if (cdev_add(&my_cdev, dev_num, 1) < 0)
    {
        printk(KERN_ERR "cdev add failed\n");
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    // Create class
    cls = class_create(CLASS_NAME);

    // Create device node /dev/mydevice
    dev = device_create(cls, NULL, dev_num, NULL, DEVICE_NAME);

    printk(KERN_INFO "Driver loaded successfully\n");
    return 0;
}

// ---------------- EXIT ----------------

static void __exit my_exit(void)
{
    device_destroy(cls, dev_num);
    class_destroy(cls);
    cdev_del(&my_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "Driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION("IOCTL Config Driver");
