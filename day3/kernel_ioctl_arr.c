#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define MAX_SIZE 100
struct arr
{
        int a[MAX_SIZE];
        int size;
        int sum;
        int avg;
};
#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct arr)

static int major;
/*ioctl handler */
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
        struct arr data;
        switch(cmd)
        {
        case IOCTL_SET_CONFIG:
                /*copy data from user */
                if(copy_from_user(&data, (struct arr __user *)arg, sizeof(data)))
                        return -EFAULT;
		for(int i=0;i<data.size;i++)
		{
			data.sum+=data.a[i];
		}
		data.avg=data.sum/data.size;
                /*copy data back to user*/
                if(copy_to_user((struct arr __user *)arg, &data, sizeof(data)))
                        return -EFAULT;
                pr_info("Kernel: sending %d %d to user\n",data.sum,data.avg);
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
