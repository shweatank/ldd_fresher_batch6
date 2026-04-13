//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'B'
struct data
{
        int mode;
        int speed;
        char name[32];
};
#define IOCTL_SET_CONFIGURATION _IOWR(IOCTL_MAGIC,1,struct data)
#define IOCTL_GET_CONFIGURATION _IOWR(IOCTL_MAGIC,2,struct data)
#define IOCTL_RESET_CONFIGURATION _IOWR(IOCTL_MAGIC,3,struct data)
static int major;
/*ioctl handler*/
struct data data;
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	switch(cmd)
	{
		case IOCTL_SET_CONFIGURATION: /*copy data from use*/
       			if(copy_from_user(&data,(struct data __user *)arg,sizeof(struct data)))
               			 return -EFAULT;
       			 pr_info("kernel: received %d %d %s from user\n",data.mode,data.speed,data.name);

			/*kernel modifies data*/
			//kernel_value=data.a+10;
			break;
		case IOCTL_RESET_CONFIGURATION:
                        data.mode=0;
                        data.speed=100;
                        strcpy(data.name,"anju");
                        break;
		case IOCTL_GET_CONFIGURATION:
			/*copy data back to user*/
			if(copy_to_user((struct data __user *)arg,&data,sizeof(struct data)))
				return -EFAULT;
			break;
		default:
			return -EINVAL;
	}
	return 0;
}
static struct file_operations fops={
	.owner=THIS_MODULE,
	.unlocked_ioctl=basic_ioctl,
};
static int __init basic_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	pr_info("basic_ioctl loaded, major=%d\n",major);
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
