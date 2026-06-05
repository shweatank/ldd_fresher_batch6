// basic_ioctl_drv.c
// // minimal IOCTL example: send an int from user ->kernel ->modify ->return

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/string.h>


struct bike{
        int mode;
        int speed;
        char name[50];
//        int result;
        };



#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_BIKE_DATA _IOWR(IOCTL_MAGIC,1,struct bike)
//#define IOCTL_CAL_OP _IOR(IOCTL_MAGIC,2,int)



static int major;
//static struct bike kernel_value;
//static int value;

//ioctl handler

static struct bike user_value;

static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
//	static struct bike user_value;

            switch(cmd){

		    case IOCTL_BIKE_DATA:

			if(copy_from_user(&user_value,(struct bike __user*)arg,sizeof(struct bike)))
				return -EFAULT;
			user_value.mode=2;
			user_value.speed=60;
			//user_value.name={"HUNTER"};
			strcpy(user_value.name,"HUNTER");

			if(copy_to_user((struct bike __user*)arg,&user_value,sizeof(struct bike)))
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


