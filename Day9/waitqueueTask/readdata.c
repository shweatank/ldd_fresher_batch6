#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

#define DEVICE_NAME "waitq_basic"

static int major;
static wait_queue_head_t wq;
static int flag = 0;
static int stop = 0;

/* READ: blocks until user triggers write() */
static ssize_t myread(struct file *file, char __user *buf,
                      size_t len, loff_t *off)
{
    char msg[] = "Hello from kernel\n";

    pr_info("Read: waiting...\n");

    wait_event_interruptible(wq, flag != 0 || stop == 1);

    if (stop) {
        pr_info("Read: exiting due to quit\n");
        return 0;   // EOF to user space
    }

    flag = 0;

    if (copy_to_user(buf, msg, sizeof(msg)))
        return -EFAULT;

    pr_info("Read: done\n");

    return sizeof(msg);
}
/* WRITE: user triggers wake-up */
static ssize_t mywrite(struct file *file,
                       const char __user *buf,
                       size_t len, loff_t *off)
{
    char kbuf[32];

    if (len > sizeof(kbuf) - 1)
        return -EINVAL;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    pr_info("Write received: %s\n", kbuf);

    /* QUIT condition */
    if (strcmp(kbuf, "quit") == 0) {
        pr_info("Stopping driver event system\n");

        stop = 1;
        flag = 1;

        /* wake ALL waiting readers */
        wake_up_interruptible(&wq);

        return len;
    }

    /* normal trigger */
    flag = 1;
    wake_up_interruptible(&wq);

    return len;
}
/* File operations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read  = myread,
    .write = mywrite,
};

/* INIT */
static int __init my_init(void)
{
    pr_info("waitq_basic driver loaded\n");

    init_waitqueue_head(&wq);

    major = register_chrdev(0, DEVICE_NAME, &fops);

    if (major < 0) {
        pr_err("Failed to register device\n");
        return major;
    }

    pr_info("Device registered with major: %d\n", major);

    pr_info("Create device node using:\n");
    pr_info("mknod /dev/%s c %d 0\n", DEVICE_NAME, major);

    return 0;
}

/* EXIT */
static void __exit my_exit(void)
{
    pr_info("waitq_basic driver unloaded\n");
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Wait queue example triggered by user write()");
