//basic_ioctl_dev.c


#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define DEVICE_NAME "calc_ioctl"
#define IOCTL_MAGIC 'A'

struct calc_data{
int a;
int b;
char op;
int result;
};

#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct calc_data)

static int major;

/*ioctl handler*/
static long calc_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
struct calc_data data;

switch(cmd){
case IOCTL_SET_VALUE:
/* copy data from user */
if(copy_from_user(&data,(struct calc_data __user*)arg, sizeof(data)))
return -EFAULT;

pr_info("Kernel: received %d %c %d from user\n",data.a,data.op,data.b);

switch(data.op)
{
case '+':
	 data.result=data.a+data.b;
	break;
	
case '-':
	data.result=data.a-data.b;
	break;

case '*':
	data.result=data.a*data.b;
	break;
	
case '/':
	if(data.b==0)
	return -EINVAL;
	data.result=data.a/data.b;
	break;
	
default:
	 return -EINVAL;
}
/* copy data back to user */
if(copy_to_user((struct calc_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;
default:
return -EINVAL;
}
return 0;
}

static struct file_operations fops={
.owner=THIS_MODULE,
.unlocked_ioctl = calc_ioctl,
};

static int __init basic_init(void){
major=register_chrdev(0,DEVICE_NAME,&fops);
pr_info("calc_ioctl loaded, major=%d\n",major);
return 0;
}
static void __exit basic_exit(void){
unregister_chrdev(major,DEVICE_NAME);
pr_info("calc_ioctl unloaded\n");
}
module_init(basic_init);
module_exit(basic_exit);

MODULE_LICENSE("GPL");
