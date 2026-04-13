//basic_ioctl_dev.c


#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define DEVICE_NAME "calc_ioctl"
#define IOCTL_MAGIC 'C'

struct calc_data{
int a;
int b;
char op;
int result;
};

#define ADD _IOWR(IOCTL_MAGIC,1,struct calc_data)
#define SUB _IOWR(IOCTL_MAGIC,2,struct calc_data)
#define MUL _IOWR(IOCTL_MAGIC,3,struct calc_data)
#define DEV _IOWR(IOCTL_MAGIC,4,struct calc_data)

static int major;

/*ioctl handler*/
static long calc_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
struct calc_data data;
//xpr_info("Kernel: received %d %c %d from user\n",data.a,data.op,data.b);

switch(cmd){
case ADD:
/* copy data from user */
if(copy_from_user(&data,(struct calc_data __user*)arg, sizeof(data)))
return -EFAULT;

data.result = data.a + data.b;

/* copy data back to user */
if(copy_to_user((struct calc_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;

case SUB:
/* copy data from user */
if(copy_from_user(&data,(struct calc_data __user*)arg, sizeof(data)))
return -EFAULT;

data.result=data.a - data.b;

/* copy data back to user */
if(copy_to_user((struct calc_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;

case MUL:
/* copy data from user */
if(copy_from_user(&data,(struct calc_data __user*)arg, sizeof(data)))
return -EFAULT;

data.result=data.a*data.b;

/* copy data back to user */
if(copy_to_user((struct calc_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;

case DEV:
/* copy data from user */
if(copy_from_user(&data,(struct calc_data __user*)arg, sizeof(data)))
return -EFAULT;

data.result=data.a/data.b;

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
