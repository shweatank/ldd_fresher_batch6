#include<linux/init.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/slab.h>
#include<linux/module.h>
#define MAGIC_NUMBER 'B'
#define DEVICE_NAME "v_con"
#define SIZE 100
struct configuration{
	int arr[SIZE];
	int len;
}kernel_value;
#define SORT_INT _IOWR(MAGIC_NUMBER,1,struct sort)
//static char *str;
static int major;
static  void sort_int(int *arr,int len)
{
	for(int i=0;i<len-1;i++)
	{	int flg=0;
		for(int j=0;j<len-1-i;j++)
		{
			if(arr[j] > arr[j+1])
			{
				flg=1;
				int a=arr[j];
				arr[j]=arr[j+1];
				arr[j+1]=a;
			}
		}
		if(!flg)
			break;
	}
}
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{
	if(cmd == SORT_INT)
	{
		if(copy_from_user(&kernel_value,(struct sort  __user*) arg,sizeof(struct sort)))
			return -EFAULT;
		sort_int(kernel_value.arr,kernel_value.len);

		if(copy_to_user((char __user*)arg,&kernel_value,SIZE))
			return -EFAULT;
	}else
	{
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
