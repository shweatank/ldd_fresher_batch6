#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "cal_ioctl"

struct array_t
{
	int arr[30];
	int size;
	int sum;
	int avg;
};

#define IOCTL_MAGIC 'A'
#define SUM_AVG_IOCTL  _IOWR(IOCTL_MAGIC, 1, struct array_t)

static int major;
static struct array_t req;

static void sum_avg( int *arr, int size )
{
	int sum = 0;
	for(int i = 0; i < size; i++)
	{
                sum += arr[i];
	}
	req.sum = sum;
	req.avg = sum / size;
}

// ioctl handler
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

	switch(cmd)
	{
		case SUM_AVG_IOCTL :
			if(copy_from_user(&req, (int __user *)arg, sizeof(struct array_t)))
				return -EFAULT;

			sum_avg( req.arr, req.size );
			
			if(copy_to_user((int __user*)arg, &req, sizeof(struct array_t))){
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


