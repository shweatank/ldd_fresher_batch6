#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static struct work_struct my_work;

static void my_work_handler(struct work_struct *work)
{
    pr_info("Work queue : Handler started\n");

    msleep(1000);

    pr_info("Work queue: Handler finished\n");
}

static int __init workq_init(void)
{
    pr_info("Workqueue module loaded\n");

    // initialize work
    INIT_WORK(&my_work, my_work_handler);

    // scheduling work
    pr_info("Workqueue: Scheduling work\n");

    schedule_work(&my_work);

    return 0;
}

static void __exit workq_exit(void)
{
    pr_info("Workqueue module exiting\n");

    flush_work(&my_work);

    pr_info("Workqueue module unloaded\n");
}

module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("Sample WORK QUEUE module");
