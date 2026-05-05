#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/kthread.h>
#include <linux/delay.h>

static struct task_struct *my_kthread;

/* =========================
   Kernel Thread Function
   ========================= */
static int thread_fn(void *data)
{
    pr_info("KTHREAD: Started\n");

    while (!kthread_should_stop()) {
        pr_info("KTHREAD: Running...\n");
        msleep(1000);  // sleep 1 second
    }

    pr_info("KTHREAD: Stopping\n");
    return 0;
}

/* =========================
   Module Init
   ========================= */
static int __init kthread_init(void)
{
    pr_info("KTHREAD: Module Loaded\n");

    my_kthread = kthread_run(thread_fn, NULL, "my_kthread");

    if (IS_ERR(my_kthread)) {
        pr_err("KTHREAD: Failed to create thread\n");
        return PTR_ERR(my_kthread);
    }

    return 0;
}

/* =========================
   Module Exit
   ========================= */
static void __exit my_kthread_exit(void)
{
    pr_info("KTHREAD: Module Unloading\n");

    if (my_kthread) {
        kthread_stop(my_kthread);
        pr_info("KTHREAD: Thread stopped\n");
    }
}

module_init(kthread_init);
module_exit(my_kthread_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Simple Kernel Thread Example");
