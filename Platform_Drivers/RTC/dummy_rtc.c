// dummy_rtc.c

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DEV_NAME "dummy_rtc"

static dev_t dev_num;
static struct cdev rtc_cdev;
static struct class *rtc_class;

struct rtc_time_data {
    int hour;
    int min;
    int sec;
};

static struct rtc_time_data rtc_time = {
    .hour = 12,
    .min  = 0,
    .sec  = 0,
};

static ssize_t rtc_read(struct file *file,
                        char __user *buf,
                        size_t len,
                        loff_t *off)
{
    char kbuf[32];
    int ret;

    if (*off)
        return 0;

    ret = snprintf(kbuf, sizeof(kbuf),
                   "%02d:%02d:%02d\n",
                   rtc_time.hour,
                   rtc_time.min,
                   rtc_time.sec);

    if (copy_to_user(buf, kbuf, ret))
        return -EFAULT;

    *off += ret;

    return ret;
}

static ssize_t rtc_write(struct file *file,
                         const char __user *buf,
                         size_t len,
                         loff_t *off)
{
    char kbuf[32];

    if (len >= sizeof(kbuf))
        len = sizeof(kbuf) - 1;

    if (copy_from_user(kbuf, buf, len))
        return -EFAULT;

    kbuf[len] = '\0';

    sscanf(kbuf, "%d:%d:%d",
           &rtc_time.hour,
           &rtc_time.min,
           &rtc_time.sec);

    pr_info("RTC Time Set: %02d:%02d:%02d\n",
            rtc_time.hour,
            rtc_time.min,
            rtc_time.sec);

    return len;
}

static const struct file_operations rtc_fops = {
    .owner = THIS_MODULE,
    .read  = rtc_read,
    .write = rtc_write,
};

static int rtc_probe(struct platform_device *pdev)
{
    int ret;

    pr_info("Dummy RTC Probe Called\n");

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    if (ret)
        return ret;

    cdev_init(&rtc_cdev, &rtc_fops);

    ret = cdev_add(&rtc_cdev, dev_num, 1);
    if (ret) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }

    rtc_class = class_create("dummy_rtc_class");
    if (IS_ERR(rtc_class)) {
        cdev_del(&rtc_cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(rtc_class);
    }

    device_create(rtc_class, NULL, dev_num,
                  NULL, DEV_NAME);

    pr_info("Dummy RTC Driver Loaded\n");

    return 0;
}

static void rtc_remove(struct platform_device *pdev)
{
    device_destroy(rtc_class, dev_num);
    class_destroy(rtc_class);

    cdev_del(&rtc_cdev);
    unregister_chrdev_region(dev_num, 1);

    pr_info("Dummy RTC Driver Removed\n");
}

static const struct of_device_id rtc_of_match[] = {
    { .compatible = "mycompany,dummy-rtc" },
    { }
};

MODULE_DEVICE_TABLE(of, rtc_of_match);

static struct platform_driver rtc_driver = {
    .probe  = rtc_probe,
    .remove = rtc_remove,
    .driver = {
        .name = "dummy_rtc",
        .of_match_table = rtc_of_match,
    },
};

module_platform_driver(rtc_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Satya");
MODULE_DESCRIPTION("Dummy RTC Platform Driver");
