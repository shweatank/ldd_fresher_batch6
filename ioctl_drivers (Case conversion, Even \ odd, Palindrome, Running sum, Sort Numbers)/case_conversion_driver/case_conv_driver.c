#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cal_ioctl"

struct CASE_CONVERSION {
    char buf[128];
};

static struct CASE_CONVERSION case_conv;

#define IOCTL_MAGIC 'A'
#define CASE_CONV  _IOWR(IOCTL_MAGIC, 1, struct CASE_CONVERSION)


static int major;

static void case_conversion(char *ptr)
{
	while(*ptr)
	{
		if(((*ptr >= 'A') && (*ptr <= 'Z')) || ((*ptr >= 'a') && (*ptr <= 'z')))
		{
			*ptr ^=32;
		}
		ptr++;
	}

}

//ioctl handler
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{

	switch(cmd)
	{
		case CASE_CONV :
			if(copy_from_user(&case_conv, (int __user *)arg, sizeof(struct CASE_CONVERSION)))
				return -EFAULT;
			pr_info("case - conversion - driver : received Data : %s  from user\n",case_conv.buf);

			//kernel modifies data
		         case_conversion(case_conv.buf);	
			//copy data back to user
			if(copy_to_user((int __user*)arg, &case_conv,sizeof(struct CASE_CONVERSION))){
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
	pr_info("case-conv-driver: loaded, major = %d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("case-conv-drv:  unloaded\n");
}

module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");


