/*IOCTL with Array Transfer
  Implement a driver that:
  Accepts an array of integers via ioctl
Calculates:
Sum
Average
Returns result to user
👉 Goal: Practice copying variable-length data using copy_from_user and copy_to_user.*/
#include <linux/module.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DEVICE_NAME "basic_ioctl"
#define IOCTL_MAGIC 'G'

#define SET_IOC_DATA _IOW(IOCTL_MAGIC , 1 , struct data)
#define GET_IOC_DATA _IOR(IOCTL_MAGIC , 2 , struct data)


static int major_number;

struct data {
	int size;
	int in_arr[20];
	int op_arr[2];
};

static long basic_ioctl(struct file *file, unsigned int command, unsigned long arg)
{

	static  struct data temp;


	int i,sum = 0,avg;


	switch (command) {
		case SET_IOC_DATA:
			if (copy_from_user(&temp, (struct data __user *)arg, sizeof(temp)))
				return -EFAULT;
			if(temp.size == 0)
				return -EFAULT;
			for(i = 0;i < temp.size; i++)
			{
				sum += temp.in_arr[i];
			}
			avg = sum/temp.size;

			temp.op_arr[0] = sum;
			temp.op_arr[1] = avg;
			printk("SUM = %d AVG = %d\n", temp.op_arr[0], temp.op_arr[1]);
			break;

		case GET_IOC_DATA:
			if (copy_to_user((struct data __user *)arg, &temp, sizeof(temp)))
				return -EFAULT;


			printk("Copied kernel to user\n");
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
MODULE_DESCRIPTION("IOCTL for given array perform the sum and aveage of elements in driver");
