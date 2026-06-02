#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/string.h>
#include<linux/mutex.h>

#define BUF_SIZE 256
#define DEVICE_NAME "control_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_MODE _IOW(IOCTL_MAGIC, 1,int)
#define IOCTL_GET_MODE _IOR(IOCTL_MAGIC, 2,int)
#define CLEAR_BUFFER      _IO(IOCTL_MAGIC, 3)
#define GET_WRITE_COUNT _IOR(IOCTL_MAGIC, 4, int)

static int major;

/*ioctl handler */

static char buffer[256];
static int device_mode=0;
static int write_count=0;

static DEFINE_MUTEX(dev_lock);

static int basic_open(struct inode* inode, struct file* file)
{
	printk(KERN_INFO "Device Opended\n");
	return 0;
}

static int basic_release(struct inode* inode,struct file* file)
{
	printk(KERN_INFO "Device Released\n");
	return 0;
}

static ssize_t basic_read(struct file *file, char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy;

        /*
         * If offset is beyond data, return 0 (EOF)
         */
        if(*offset >= count)
                return 0;

        bytes_to_copy = min(count, (size_t)(count- *offset));

        /*
         * Copy data from kernel space to user space
         */
        if(copy_to_user(user_buffer,buffer + *offset,bytes_to_copy))
                return -EFAULT;

        *offset += bytes_to_copy;

        printk(KERN_INFO "basic_char: read %d bytes\n",bytes_to_copy);
        return bytes_to_copy;
}

static ssize_t basic_write(struct file *file,const char __user *user_buffer, size_t count, loff_t *offset)
{
        int bytes_to_copy;
        bytes_to_copy = min(count, (size_t) BUF_SIZE-1);

        /*
         * Copy data from user space to kernel space
         */
        if(bytes_to_copy > 256)
	{
		bytes_to_copy=256;
	}
	mutex_lock(&dev_lock);
        if(copy_from_user(buffer,user_buffer,bytes_to_copy))
	{
		mutex_unlock(&dev_lock);
                return -EFAULT;
	}

	write_count++;

	mutex_unlock(&dev_lock);


        printk(KERN_INFO "basic_char: wrote %d bytes\n",bytes_to_copy);
        return bytes_to_copy;
}

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	//struct Operation user_value;

	switch(cmd)
	{
		case IOCTL_SET_MODE:

			if(copy_from_user(&device_mode,(int __user * ) arg, sizeof(int)))
				return -EFAULT;

			pr_info("Kernel: received %d from user\n",device_mode);

			//kernel_value = user_value;
			break;

		case IOCTL_GET_MODE:

			if(copy_to_user((int __user *)arg, &device_mode, sizeof(int)))
				return -EFAULT;
			break;

		case CLEAR_BUFFER:

			memset(&buffer,0,sizeof(buffer));
			break;

		case GET_WRITE_COUNT:
			if(copy_to_user((int __user *)arg,&write_count,sizeof(int)))
				return -EFAULT;
			break;

		default:
			return -EINVAL;
	}
	return 0;
}

static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,
	.open = basic_open,
	.read = basic_read,
	.write = basic_write,
	.release = basic_release,
};

static int __init basic_init(void)
{
	major = register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic ioctl loaded, major=%d\n",major);
	mutex_init(&dev_lock);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major, DEVICE_NAME);
	pr_info("basic_ioctl unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
