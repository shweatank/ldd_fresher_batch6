#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
struct st
{
	int a,b,result;
	char c;
};

#define DEVICE_NAME "basic ioctl"
#define IOCTL_MAGIC 'B'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct st)
static int major;
static int kernel_value=0;
static volatile struct st kernel_val={0,0,0};

static long basic_ioctl(struct file*file , unsigned int cmd,unsigned long arg)
{
	struct st user_value;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
		if(copy_from_user(&user_value,(struct st __user*)arg,sizeof(struct st)))
		return -EFAULT;
		
		pr_info("Kernel: Received %d %d from user\n",user_value.a,user_value.b);
		// Kernel Modifies the data
		//kernel_value=user_value+10;
		kernel_val.a=user_value.a;
		kernel_val.b=user_value.b;
		kernel_val.c=user_value.c;
		pr_info("Kernel: In kernel Values %d %d %d\n",kernel_val.c,kernel_val.b,kernel_val.c);		
		/*kernel_val.a+=10;
		kernel_val.b+=10;
		kernel_val.c+=10;*/
		kernel_val.ch=user_value.ch;
		switch (kernel_val.ch)
			case '+':
				kernel_val.result=kernel_val.a+kernel_val.b;
				break;
			case '-':
				kernel_val.result=kernel_val.a-kernel_val.b;
				break;
			case '*':
				kernel_val.result=kernel_val.a*kernel_val.b;
				break;
			case '/':
				if(kernel_val.b==0)
					return -EFAULT;
				else
				kernel_val.result=kernel_val.a/kernel_val.b;
				break;
				
		//Copy back to user 
		if(copy_to_user((struct st __user*)arg,&kernel_val,sizeof(struct st)))
			return -EFAULT;
	break;
		default:
			return -EFAULT;
	}
return 0;
}
static struct file_operations fops={
					.owner=THIS_MODULE,
					.unlocked_ioctl=basic_ioctl
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
MODULE_AUTHOR("Satheesh Kumar");
//MODULE_DESCRIPTION("Basic IOCTL");







