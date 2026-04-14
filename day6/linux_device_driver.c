#include<linux/module.h> //Core module macros			 
#include<linux/kernel.h> //printk()
#include<linux/init.h>   // __init,__exit
#include<linux/fs.h>  //register_chrdev,file_operations
#include<linux/uaccess.h> //copy_to_user,copy_from_user
#include<linux/mutex.h>

#define DEVICE_NAME "my_ioctl_dev"
#define BUF_SIZE 256
#define IOCTL_MAGIC 'A'
#define IOCTL_SET_MODE _IOW(IOCTL_MAGIC,1,int)
#define IOCTL_GET_MODE _IOR(IOCTL_MAGIC,2,int)
#define IOCTL_CLEAR_BUFFER _IO(IOCTL_MAGIC,3)
#define IOCTL_GET_WCOUNT  _IOR(IOCTL_MAGIC,4,int)

static int major_number;
static char kernel_buffer[BUF_SIZE];
static int device_mode;
static int write_count;
static DEFINE_MUTEX(dev_lock);

/*
 * Called when user opens /dev/basic_char
 */

static int basic_open(struct inode *inode,struct file *file)
{
	//printk(KERN_INFO "basic_char:device opened\n");
	printk(KERN_INFO "I am opened\n");
	return 0;
}

/*
 * called when user closes /dev/basic_char
 */

static int basic_release(struct inode *inode,struct file *file)
{
	//printk(KERN_INFO "basic_char:device closed\n");
	printk(KERN_INFO "I am closed\n");
	return 0;
}

/*
 * Called when user reads from /dev/basic_char
 */

static ssize_t basic_read(struct file *file,char __user *user_buffer,size_t count,loff_t *offset)
{
	size_t bytes_to_copy;
	if(*offset>=BUF_SIZE)
		return 0;
	bytes_to_copy=min(count,(size_t)BUF_SIZE);
	mutex_lock(&dev_lock);
	/*
	 * copy data from kernel space to user space
	 */
	if(copy_to_user(user_buffer,kernel_buffer,bytes_to_copy))
	{
		mutex_unlock(&dev_lock);
		return -EFAULT;
	}
	mutex_unlock(&dev_lock);
	*offset+=bytes_to_copy;
	printk(KERN_INFO "basic_char :read %zu bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

/*
 * called when user writes to /dev/basic_char
 */

static ssize_t basic_write(struct file *file,const char __user *user_buffer,size_t count,loff_t *offset)
{
	int bytes_to_copy;
	bytes_to_copy=min(count,(size_t)BUF_SIZE);

	/*
	 * copy data from user space to kernel space
	 */
	mutex_lock(&dev_lock);
	if(copy_from_user(kernel_buffer,user_buffer,bytes_to_copy))
	{
		mutex_unlock(&dev_lock);
		return -EFAULT;
	}
	write_count++;
	mutex_unlock(&dev_lock);
	printk(KERN_INFO "basic_char:write %d bytes\n",bytes_to_copy);
	return bytes_to_copy;
}

static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        int temp;
	mutex_lock(&dev_lock);
        switch(cmd)
        {
        case IOCTL_SET_MODE:
                /*copy data from user */
                if(copy_from_user(&temp, (int __user *)arg, sizeof(int)))
		{
			mutex_unlock(&dev_lock);
                        return -EFAULT;
		}
		device_mode=temp;
                pr_info("Mode set to %d\n", device_mode);
		break;
	case IOCTL_GET_MODE:
                /*copy data back to user*/
                if(copy_to_user((int __user *)arg, &device_mode, sizeof(int)))
		{
			mutex_unlock(&dev_lock);
                        return -EFAULT;
		}
                break;
	case IOCTL_CLEAR_BUFFER:
		memset(kernel_buffer,0,BUF_SIZE);
		printk(KERN_INFO "Buffer cleared\n");
		break;
	case IOCTL_GET_WCOUNT:
		if(copy_to_user((int __user *)arg,&write_count,sizeof(int)))
		{
			mutex_unlock(&dev_lock);
			return -EFAULT;
		}
		break;
        default:
		mutex_unlock(&dev_lock);
                return -EINVAL;
        }
	mutex_unlock(&dev_lock);
        return 0;
}

/*
 * file operations structure
 * this connects system calls to driver functions
 */

static struct file_operations basic_fops={
	.owner=THIS_MODULE,
	.open=basic_open,
	.read=basic_read,
	.write=basic_write,
	.release=basic_release,
	.unlocked_ioctl=basic_ioctl,
};
/*
 * module instructions
 */
static int __init basic_char_init(void)
{
	/*
	 * register character device
	 * 0 ->dynamic major number
	 */
	major_number=register_chrdev(0,DEVICE_NAME,&basic_fops);
	if(major_number<0)
	{
		printk(KERN_INFO "basic_char:faile dto register device\n");
		return major_number;
	}
	printk(KERN_INFO "basic_char:loaded\n");
	printk(KERN_INFO "basic_char:major number=%d\n",major_number);
	printk(KERN_INFO "Create device node with:\n");
	printk(KERN_INFO "mknod /dev/%s c %d 0\n",DEVICE_NAME,major_number);
	return 0;
}
/*
 * module cleanup
 */
static void __exit basic_char_exit(void)
{
	unregister_chrdev(major_number,DEVICE_NAME);
	printk(KERN_INFO "basic_char:unloaded\n");
}
/*kernel module macros*/
module_init(basic_char_init);
module_exit(basic_char_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("sarika");
MODULE_DESCRIPTION("Educational basic character driver with file operations");
