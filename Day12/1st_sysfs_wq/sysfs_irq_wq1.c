#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/sched.h>

static int value = 0;
static int event_occurred = 0;
static struct kobject *my_kobj;

/* 1. Initialize the Wait Queue */
static DECLARE_WAIT_QUEUE_HEAD(my_wait_queue);

/* 2. Define the Workqueue Structure */
struct work_data {
    struct work_struct my_work;
    int new_val;
};
static struct work_data *irq_work;

/* Workqueue Handler (Bottom Half) */
static void work_handler(struct work_struct *work) {
    struct work_data *data = container_of(work, struct work_data, my_work);
    
    value = data->new_val; // Update the stored value
    event_occurred = 1;    // Set condition for wait queue
    
    pr_info("Workqueue: Value updated to %d. Waking up waiters.\n", value);
    
    /* Wake up processes waiting on the queue */
    wake_up_interruptible(&my_wait_queue);
    
    /* Notify sysfs of the change (allows userspace poll() to work) */
    sysfs_notify(my_kobj, NULL, "my_value");
}

/* 3. Interrupt Handler (Top Half) */
static irqreturn_t my_interrupt_handler(int irq, void *dev_id) {
    pr_info("Interrupt: Scheduling workqueue.\n");
    irq_work->new_val = value + 1; // Example: increment value
    schedule_work(&irq_work->my_work);
    return IRQ_HANDLED;
}

/* 4. Sysfs Callbacks */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
    /* Processes reading this file can be made to wait for an update */
    wait_event_interruptible(my_wait_queue, event_occurred == 1);
    event_occurred = 0; // Reset after reading
    return sprintf(buf, "%d\n", value);
}

static struct kobj_attribute value_attribute = __ATTR(my_value, 0660, value_show, NULL);

static int __init my_module_init(void) {
    int error = 0;

    /* Create kobject in /sys/kernel/sysfs_example */
    my_kobj = kobject_create_and_add("sysfs_example", kernel_kobj);
    if (!my_kobj) return -ENOMEM;

    error = sysfs_create_file(my_kobj, &value_attribute.attr);
    if (error) {
        kobject_put(my_kobj);
        return error;
    }

    /* Initialize work item */
    irq_work = kmalloc(sizeof(struct work_data), GFP_KERNEL);
    INIT_WORK(&irq_work->my_work, work_handler);

    /* For a real device, you would use request_irq(IRQ_LINE, my_interrupt_handler, ...) here */
    pr_info("Module loaded. Value file at /sys/kernel/sysfs_example/my_value\n");
    return 0;
}

static void __exit my_module_exit(void) {
    cancel_work_sync(&irq_work->my_work);
    kfree(irq_work);
    kobject_put(my_kobj);
    pr_info("Module unloaded.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");

