#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

struct device_config
{
	int mode;
	int speed;
	char name[32];
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_CONFIG _IOW(IOCTL_MAGIC,1,struct device_config)
#define IOCTL_GET_CONFIG _IOR(IOCTL_MAGIC,2,struct device_config)
#define IOCTL_RESET_CONFIG _IO(IOCTL_MAGIC,3)

static int major;
static struct device_config dev_cfg={
	.mode=0,
	.speed=100,
	.name="default"
};
/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct device_config data;
        switch(cmd)
        {
        case IOCTL_SET_CONFIG:
                /*copy data from user */
                if(copy_from_user(&data, (struct device_config __user *)arg, sizeof(data)))
                        return -EFAULT;
		dev_cfg=data;
                pr_info("Kernel: received %d %d %s from user\n",dev_cfg.mode,dev_cfg.speed,dev_cfg.name);
		break;
	case IOCTL_GET_CONFIG:
                /*copy data back to user*/
                if(copy_to_user((struct device_config __user *)arg, &dev_cfg, sizeof(dev_cfg)))
                        return -EFAULT;
                pr_info("Kernel: sending %d %d %s to user\n",dev_cfg.mode,dev_cfg.speed,dev_cfg.name);
                break;
	case IOCTL_RESET_CONFIG:
		dev_cfg.mode=0;
		dev_cfg.speed=100;
		strcpy(dev_cfg.name,"default");
		pr_info("Reset done\n");
		break;
	default:
                return -EINVAL;
        }
        return 0;
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .unlocked_ioctl = basic_ioctl,
};

static int __init basic_init(void)
{
        major = register_chrdev(0, DEVICE_NAME, &fops);
        pr_info("basic_ioctl loaded, major =%d\n", major);
	return 0;
}
static void __exit basic_exit(void)
{
   unregister_chrdev(major,DEVICE_NAME);
   pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nandini");
MODULE_DESCRIPTION(" IOCTL this module is for educational purpose");
