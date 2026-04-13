/* kernel program to set and get data through ioctl and perform calculation in thread irq */
#include<linux/module.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/fs.h>
#include<linux/uaccess.h>

#define DEVICE_NAME "basic_ioctl"
#define IRQ_NUM 1
#define IOCTL_MAGIC 'A'
#define SET_DATA _IOW(IOCTL_MAGIC,1,struct calc_data *)
#define GET_DATA _IOR(IOCTL_MAGIC,2,struct calc_data *)

struct calc_data{
int a;
int b;
char op;
int result;
};
static struct calc_data data;

static int major;

/*ioctl handler*/
static long basic_ioctl(struct file *file,unsigned int cmd,unsigned long arg){
switch(cmd){
case SET_DATA:
        if(copy_from_user(&data,(struct calc_data *)arg, sizeof(data)))
	return -EFAULT;
        pr_info("data received : %d %c %D \n",data.a,data.op,data.b);
        break;
case GET_DATA:
        if(copy_to_user((struct calc_data *)arg,&data, sizeof(data)))
	return -EFAULT;
        break;
default:
	return -EINVAL;
}
return 0;

}

/*irq handlers */
static irqreturn_t irq_top(int irq, void *dev_id)
{
return IRQ_WAKE_THREAD;
}

static irqreturn_t irq_thread(int irq,void *dev_id){
pr_info("THreaded irq handler(can sleep): \n");
msleep(100);  //simulate delay

switch(data.op){
case '+': data.result=data.a+data.b;
        break;
case '-':data.result=data.a-data.b;
        break;
case 'x': data.result=data.a*data.b;
        break;
case '/': if(data.b!=0)
        data.result=data.a/data.b;
        else
        pr_info("Devision by zero\n");
        break;
default: pr_info("Invalid operator!\n");

}
pr_info("Result: %d\n",data.result);
return IRQ_HANDLED;
}

/*file ops*/

static struct file_operations fops={
.owner=THIS_MODULE,
.unlocked_ioctl = basic_ioctl,
};

static int __init irq_threaded_init(void)
{
int ret;
major=register_chrdev(0,DEVICE_NAME,&fops);
if(major<0)
{
pr_err("Device registration failed!\n");
return major;
}
pr_info("device registered with major=%d\n",major);
ret=request_threaded_irq(IRQ_NUM,irq_top,irq_thread,IRQF_SHARED,"irq_threadd",(void*)irq_thread);
if(ret){
pr_err("IRQ Request failed!\n");
unregister_chrdev(major,DEVICE_NAME);
return ret;
}
pr_info("Device loaded successfully!\n");
return 0;
}

static void __exit irq_threaded_exit(void)
{

free_irq(IRQ_NUM,(void*)irq_thread);
unregister_chrdev(major,DEVICE_NAME);
pr_info("Driver unloaded\n");
}

module_init(irq_threaded_init);
module_exit(irq_threaded_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("VAISHNAVI");

