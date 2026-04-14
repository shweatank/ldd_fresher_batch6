/* simple workqueue example */
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/workqueue.h>
#include<linux/delay.h>

static struct work_struct my_work;

/* Work handler function */
static void my_work_handler(struct work_struct *work)
{
pr_info("Workqueue: handler started\n");

/* Simulate some work (sleep allowed ) */
msleep(2000);

pr_info("Workqueue handler finished\n");
}

/* MOdule init */
static int __init workq_init(void)
{
pr_info("Workqueue module loaded\n");

/* Initialize work */
INIT_WORK(&my_work,my_work_handler);

/* Schedule work */
pr_info("Workqueue: Scheduling work\n");
schedule_work(&my_work);

return 0;
}

/* Module exit */
static void __exit workq_exit(void)
{
pr_info("Workqueue module exiting\n");
/* Ensure work is completed before exit */
flush_work(&my_work);
pr_info("Workqueue module unloaded\n");
}
module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("VAISHNAVI");
MODULE_DESCRIPTION("Simple Workqueue Example for Linux kernel 6.17");

