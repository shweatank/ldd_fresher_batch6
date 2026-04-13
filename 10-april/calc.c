//basic_ioctl_drv.c
//Minimal IOCTL example: send an int from user ->kernel-> modify->return
#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#define DEVICE_NAME "basic_ioctl"
struct data
{
	int a;
	int b;
	int res;
	char str[10];
};
#define IOCTL_MAGIC 'A'
#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct data)
static int major;
/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
	struct data data;
	switch(cmd)
	{
		case IOCTL_SET_VALUE:
			/*copy data from use*/
			if(copy_from_user(&data,(struct data __user *)arg,sizeof(data)))
				return -EFAULT;
			pr_info("kernel: received %d %d %s from user\n",data.a,data.b,data.str);
			/*kernel modifies data*/
			char symbol;
			if(strcmp(data.str,"add")==0)
			{
				symbol='+';

			}
			else if(strcmp(data.str,"sub")==0)
			{
				symbol='-';
			}
			else if(strcmp(data.str,"mul")==0)
			{
				symbol='*';
			}
			else if(strcmp(data.str,"div")==0)
			{
				symbol='/';
			}
			switch(symbol)
			{
				case '+':data.res=data.a+data.b;
					 break;
				case '-':data.res=data.a-data.b;
					 break;
				case '*':data.res=data.a*data.b;
					 break;
				case '/':data.res=data.a/data.b;
					 break;
			}
			/*copy data back to user*/
			if(copy_to_user((struct data __user *)arg,&data,sizeof(data)))
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
