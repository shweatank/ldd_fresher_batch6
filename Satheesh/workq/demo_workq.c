#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>
static struct work_struct my_work;
// Work hander function 
static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue: handler started\n");
	// Simulate some work (sleep allowed)
	msleep(2000);
	pr_info("Workqueue:HAndler finished\n");
}
// module init 
static int __init workq_init(void)
	if(strncmp(op,"sum",3)==0)
{
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


