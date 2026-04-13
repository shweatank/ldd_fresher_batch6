#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define DEVICE_NAME "calc_ioctl"
#define IOCTL_MAGIC 'D'

struct calc_data{
int a;
int b;
char op;
};

#define IOCTL_SET_VALUE _IOWR(IOCTL_MAGIC,1,struct calc_data)
#define IOCTL_GET_VALUE _IOWR(IOCTL_MAGIC,2,int)
static int major;
static int result;

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
         result=data.a+data.b;
        break;

case '-':
        result=data.a-data.b;
        break;

case '*':
        result=data.a*data.b;
        break;

case '/':
        if(data.b==0)
        return -EINVAL;
        result=data.a/data.b;
        break;

default:
         return -EINVAL;
}
break;

case IOCTL_GET_VALUE:

/* copy data back to user */
if(copy_to_user((int __user *)arg,&result,sizeof(int)))
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
