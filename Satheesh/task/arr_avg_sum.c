#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/module.h>
#define MAGIC_NUMBER 'B'
#define DEVICE_NAME "SUM_AVG"
#define SIZE 100
void cal_sum(int *,int);
struct sort
{
	int arr[SIZE];
	int len;
}kernel_value;
#define ARR_WRITE _IOW(MAGIC_NUMBER,1,struct sort)
#define ARR_SUM _IOR(MAGIC_NUMBER,2,int) 
#define ARR_AVG _IOR(MAGIC_NUMBER,3,int)
//static char *str;
static int major;
static int average,sum;
void cal_sum(int *arr,int n)
{
	sum=0,average=0;
	for(int i=0;i<n;i++)
		sum+=arr[i];
	average=sum/n;
}
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{
	switch (cmd)
	{
		case ARR_WRITE:
		if(copy_from_user(&kernel_value,(struct sort __user*)arg,sizeof(struct sort)))
			return -EFAULT;
		pr_info("Written array into kernel\n");
		break;
		case ARR_SUM:
			cal_sum(kernel_value.arr,kernel_value.len);
			if(copy_to_user((int __user*)arg,&sum,sizeof(int)))
				return -EFAULT;
			break;
		case ARR_AVG:
			cal_sum(kernel_value.arr,kernel_value.len);
			if(copy_to_user((int __user*)arg,&average,sizeof(int)))
				return -EFAULT;
			break;
		default:
			return -EFAULT;
	}
return 0;
}
struct file_operations fops={
		.owner=THIS_MODULE,
		.unlocked_ioctl=my_ioctl
};

static int __init case_convert_init(void)
{
	major=register_chrdev(0,DEVICE_NAME,&fops);
	//str=kmalloc(100*sizeof(char),GFP_KERNEL);
	/*if(!str)
	{
		printk(KERN_ERR"Failed to allocate memory\n");
		return -ENOMEM;
	}*/
	if(major<0)
	{
		printk(KERN_ALERT"Cannot create driver\n");
		return -EFAULT;
	}
	pr_info("case converter loaded major = %d \n",major);
	
return 0;
}
static void __exit case_convert_exit(void)
{
	unregister_chrdev(major,DEVICE_NAME);
	pr_info("case converter unloaded");
//	kfree(str);
}
module_init(case_convert_init);
module_exit(case_convert_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("chintubhai");
//MODULE_DESCRIPTION("Case converter using ioctl");
