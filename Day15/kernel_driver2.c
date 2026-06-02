#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mutex.h>

#define DEVICE_NAME "my_mutex_dev"

static DEFINE_MUTEX(dev_mutex);
static int device_config = 0;
static int major;
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    mutex_lock(&dev_mutex);

    pr_info("Config before: %d\n", device_config);

    device_config = arg;  // critical section

    pr_info("Config updated to: %d\n", device_config);

    mutex_unlock(&dev_mutex);

    return 0;
}

static struct file_operations fops = {
    .unlocked_ioctl = my_ioctl,
};

static int __init my_init(void)
{
    major=register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("Mutex driver loaded,major=%d\n",major);
    return 0;
}

static void __exit my_exit(void)
{
    unregister_chrdev(0, DEVICE_NAME);
}
module_init(my_init);
module_exit(my_exit);
MODULE_LICENSE("GPL");
