#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>
#include "config_ioctl.h"

#define DEVICE_NAME "config_dev"

static int major;
// Persistent kernel-side state
static struct device_config current_cfg = {1, 115200, "Default_Dev"};

static long dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    struct device_config tmp_cfg;

    switch(cmd) {
        case SET_CONFIG:
            if (copy_from_user(&current_cfg, (struct device_config __user *)arg, sizeof(struct device_config)))
                return -EFAULT;
            pr_info("Config updated: Mode=%d, Speed=%d, Name=%s\n", current_cfg.mode, current_cfg.speed, current_cfg.name);
            break;

        case GET_CONFIG:
            if (copy_to_user((struct device_config __user *)arg, &current_cfg, sizeof(struct device_config)))
                return -EFAULT;
            break;

        case RESET_CONFIG:
            current_cfg.mode = 1;
            current_cfg.speed = 9600;
            strcpy(current_cfg.name, "Reset_Dev");
            pr_info("Config reset to defaults\n");
            break;

        default:
            return -EINVAL;
    }
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = dev_ioctl,
};

static int __init cfg_init(void) {
    major = register_chrdev(0, DEVICE_NAME, &fops);
    pr_info("Config driver loaded. Major: %d\n", major);
    return 0;
}

static void __exit cfg_exit(void) {
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(cfg_init);
module_exit(cfg_exit);
MODULE_LICENSE("GPL");

