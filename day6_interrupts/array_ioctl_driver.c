#include<linux/module.h>
#include<linux/fs.h>
#include<linux/uaccess.h>
#include<linux/device.h>

#define DEVICE_NAME "array_ioctl"
#define IOCTL_MAGIC 'C'
#define IOCTL_ARRAY_SUM _IOWR(IOCTL_MAGIC,1,struct array_data)
#define MAX_SIZE 100

struct array_data{
int a[MAX_SIZE];
int size;
int sum;
int
 avg;
};


static int major;

/*ioctl handler*/
static long array_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
struct array_data data;

switch(cmd){
case IOCTL_ARRAY_SUM:
/* copy data from user */
if(copy_from_user(&data,(struct array_data __user*)arg, sizeof(data)))
return -EFAULT;

/* validate size */
if(data.size<=0 || data.size>MAX_SIZE)
{
printk("Invalid size\n");
return -EINVAL;
}

data.sum=0;
for(int i=0;i<data.size;i++){
data.sum+=data.a[i];
}

data.avg = data.sum/data.size;

/* copy data back to user */
if(copy_to_user((struct array_data __user *)arg,&data,sizeof(data)))
return -EFAULT;

break;
default:
return -EINVAL;

}

return 0;
}

static struct file_operations fops={
.owner=THIS_MODULE,
.unlocked_ioctl = array_ioctl,
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
