#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>

static struct work_struct my_work;

//work handler function
static void my_work_handler(struct work_struct *work)
{
	pr_info("Workqueue : handler started\n");

	//simulate some work (sleep allowed)

	pr_info("workqueue:handler finished\n");

}

//module init

static int __init workq_init(void)
{
	pr_info("workqueue module loaded\n");
	//initialize work
	INIT_WORK(&my_work,my_work_handler);

	//schedule work
	pr_info("workqueue:scheduling  woek\n");
	schedule_work(&my_work);
	return 0;
}

//module exit
static void __exit workq_exit(void)
{
	pr_info("Workqueue module exiting\n");
	//ensure work is completed before exit
	flush_work(&my_work);
	pr_info("Workqueue module unloaded\n");

}

module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("PRUDHVI");
MODULE_DESCRIPTION("simple workqueue example for linux kenel 6.8");
