/*Design a driver that manages a device configuration struct:
Structure contains:
int mode
int speed
char name[32]
IOCTL commands:
Set configuration
Get configuration
Reset to default
👉 Goal: Handle structured data exchange via ioctl.*/
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include<linux/string.h>
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'G'

#define SET_IOC_DATA _IOW(IOCTL_MAGIC , 1 , struct config)
#define GET_IOC_DATA _IOR(IOCTL_MAGIC , 2 , struct config)
#define RESET_IOC_DATA _IOR(IOCTL_MAGIC,3,struct config)

static int major_number;

struct config {
	int mode;
	int speed;
	char name[32];
};

static long basic_ioctl(struct file *file, unsigned int command, unsigned long arg)
{

	static  struct config temp;


	switch (command) {
		case SET_IOC_DATA:
			if (copy_from_user(&temp, (struct config __user *)arg, sizeof(temp)))
				return -EFAULT;
			temp.mode = 5;
			temp.speed = 100;
			strcpy(temp.name,"Gangadhar");
			printk(KERN_INFO "The changing the mode and speed to %d and %d \n",temp.mode,temp.speed);
			break;

		case GET_IOC_DATA:
			if (copy_to_user((struct config __user *)arg, &temp, sizeof(temp)))
				return -EFAULT;
		

			printk("Copied kernel to user\n");
			break;
		case RESET_IOC_DATA:
			temp.mode = 0;
			temp.speed = 0;
			strcpy(temp.name,"Device");
			printk("Reseted the device\n");
			if (copy_to_user((struct config __user *)arg, &temp, sizeof(temp)))
                                 return -EFAULT;

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
	major_number = register_chrdev(0, DEVICE_NAME, &fops);
	pr_info("basic_ioctl loaded, major = %d\n", major_number);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major_number, DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("IOCTL SET , GET ,RESET DATA");
