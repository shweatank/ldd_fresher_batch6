// basic_ioctl_drv.c
// // minimal IOCTL example: send an int from user ->kernel ->modify ->return

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>


struct cal{
        int a;
        int b;
        char op;
//        int result;
        };



#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_CAL_IP _IOW(IOCTL_MAGIC,1,struct cal)
#define IOCTL_CAL_OP _IOR(IOCTL_MAGIC,2,int)



static int major;
static struct cal kernel_value;
static int value;

//ioctl handler

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	static struct cal user_value;

            switch(cmd){

		    case IOCTL_CAL_IP:

			if(copy_from_user(&user_value,(struct cal __user*)arg,sizeof(struct cal)))
				return -EFAULT;
			char op=user_value.op;
			switch(op){
			
				case '+':
					value = user_value.a + user_value.b;
					break;
				case '-':
                                        value = user_value.a - user_value.b;
                                        break;
				case '*':
                                        value = user_value.a * user_value.b;
                                        break;
				case '/':
                                        value = user_value.a / user_value.b;
                                        break;
	
		default:
			return -EINVAL;
	
	}
		//	break;
	case IOCTL_CAL_OP:
	if(copy_to_user((int __user*)arg,&value,sizeof(int)))
                return -EFAULT;
	break;

	default:
	return -EINVAL;
                        
	    }
	return 0;


}


static struct file_operations fops=
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = basic_ioctl,

};

static int __init basic_init(void)
{
	major=register_chrdev(0, DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded,major=%d\n",major);
	return 0;
}

static void __exit basic_exit(void)
{
       unregister_chrdev(major, DEVICE_NAME);
        pr_info("basic_ioctl unloaded\n");
//return 0;
}


module_init(basic_init);
module_exit(basic_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("pavan");
MODULE_DESCRIPTION("this is a driver to illustrate the usage of ioctl command");


