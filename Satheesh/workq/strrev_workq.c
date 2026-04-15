#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
static struct work_struct my_work;
// Work hander function 
char *str;
int major;
#define DEVICE_NAME "workq"
#define MAGIC_NUMBER 'V'
#define SND_STR _IOW(MAGIC_NUMBER,1,char *)

static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue: handler started\n");
	// Simulate some work (sleep allowed)
	//msleep(2000)
	int len=0;
	while(str[len++]);
	for(int i=0,j=len;str[i];i++)
	{
				
	}
	pr_info("Workqueue:HAndler finished\n");
}
static long my_ioctl(struct file*file,unsigned int cmd,unsigned long arg)
{
	if(cmd==SND_STR)
		if(copy_from_user(str,(char __user*)arg,49))
			return -EFAULT;
}
// module init 
struct file_operations fops={.owner=THIS_MODULE,.unlocked_ioctl=my_ioctl};
static int __init workq_init(void)
{
	str=(char *)kmalloc(50*sizeof(char));
	if(!str)
	{
		pr_err("Failed to allocate memory\n");
		return -ENOMEM;
	}

	pr_info("Workqueue module loaded\n");
	INIT_WORK(&my_work,my_work_handler);
	//schedule work
	pr_info("Workqueue: Scheduling work\n");
	schedule_work(&my_work);
	return 0;
}
//Module exit 
static void __exit workq_exit(void)
{
	pr_info("Workqueue module exiting\n");
	//ensure work is sompleted before exiting 
	flush_work(&my_work);
	pr_info("Workqueue module loaded\n");
}
module_init(workq_init);
module_exit(workq_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("SatheeshKumar");
MODULE_DESCRIPTION("Simple workq simulation");


