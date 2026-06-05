// my_semaphore_driver.c

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "my_sema_dev"
#define MAJOR_NUM 240

// IOCTL commands
#define SET_CONFIG _IOW('a', 'a', int)

static struct semaphore dev_sema;
static int device_config = 0;

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

        // Acquire semaphore (blocking)
        if (down_interruptible(&dev_sema)) {
            pr_err("Semaphore interrupted\n");
            return -ERESTARTSYS;
        }

        pr_info("Config before: %d\n", device_config);

        device_config = value;   // critical section

        pr_info("Config updated to: %d\n", device_config);

        // Release semaphore
        up(&dev_sema);

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
    // Initialize semaphore with count = 1 (binary semaphore)
    sema_init(&dev_sema, 1);

    if (register_chrdev(MAJOR_NUM, DEVICE_NAME, &fops) < 0) {
        pr_err("Failed to register device\n");
        return -1;
    }

    pr_info("Semaphore driver loaded\n");
    pr_info("Create device: mknod /dev/%s c %d 0\n", DEVICE_NAME, MAJOR_NUM);

    return 0;
}

/* ================= EXIT ================= */
static void __exit my_exit(void)
{
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    pr_info("Semaphore driver unloaded\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
