#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cal_ioctl"

static int result;

#define IOCTL_MAGIC 'A'
#define EVEN_ODD_IOCTL  _IOWR(IOCTL_MAGIC, 1, int)


static int major;

//ioctl handler
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

	int num;
	switch(cmd)
	{
		case EVEN_ODD_IOCTL :
			if(copy_from_user(&num, (int __user *)arg, sizeof(int)))
				return -EFAULT;
			pr_info("recieved number  = %d\n",num);

			//kernel modifies data
		         result = num & 1;	 
			//copy data back to user
			if(copy_to_user((int __user*)arg, &result, sizeof(int))){
			  return -EFAULT;
			}
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
	major = register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("even odd driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("even odd -drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


