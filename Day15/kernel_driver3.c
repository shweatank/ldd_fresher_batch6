// my_mutex_driver.c

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "my_mutex_dev"
#define MAJOR_NUM 0

// IOCTL command
#define SET_CONFIG _IOW('a', 'a', int)

static DEFINE_MUTEX(dev_mutex);
static int device_config = 0;
static int major;
/* ================= IOCTL ================= */
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int value;

    switch (cmd) {
    case SET_CONFIG:

        if (copy_from_user(&value, (int __user *)arg, sizeof(value))) {
            pr_err("Failed to copy from user\n");
            return -EFAULT;
        }

        mutex_lock(&dev_mutex);

        pr_info("Config before: %d\n", device_config);

        device_config = value;   // critical section

        pr_info("Config updated to: %d\n", device_config);

        mutex_unlock(&dev_mutex);

        break;

    default:
        return -EINVAL;
    }

    return 0;
}

/* ================= FOPS ================= */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = my_ioctl,
};

/* ================= INIT ================= */
static int __init my_init(void)
{
    if ((major=register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops)) < 0) {
        pr_err("Failed to register device\n");
        return -1;
    }

    pr_info("Mutex driver loaded\n");
    pr_info("Create device: mknod /dev/%s c %d\n", DEVICE_NAME,major);

    return 0;
}

/* ================= EXIT ================= */
static void __exit my_exit(void)
{
    unregister_chrdev(major, DEVICE_NAME);
    pr_info("Mutex driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
