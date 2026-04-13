#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/interrupt.h>
#include<linux/delay.h>


#define IRQ_NUM 1
#define DEVICE_NAME "calc_device"


static struct calc_data{
    char op;
    int x;
    int y;
    bool data_ready;  //this is a member in struct to know whether the data is collected from the user or not
}my_data;

static int major_number;
static irqreturn_t irq_top(int irq , void *dev_id)
{
    return IRQ_WAKE_THREAD;   //waking up the calculator thread whenever a interrupt occurs
}

static irqreturn_t irq_thread(int irq , void *dev_id)  //this runs in the process context
{
    pr_info("Threaded IRQ Handler (can sleep)\n");

    // msleep(50);  //irq thread can sleep

    int result = 0;

    if (!my_data.data_ready) {
        pr_info("Calc Driver: Interrupt occurred, but no numbers provided yet!\n");
        return IRQ_HANDLED;
    }

    switch (my_data.op) {
        case '+': 
            result = my_data.x + my_data.y; 
            break;
        case '-': 
            result = my_data.x - my_data.y; 
            break;
        case '*': 
            result = my_data.x * my_data.y; 
            break;
        case '/': 
            if (my_data.y != 0)     
                result = my_data.x / my_data.y; 
        
            break;

        default: 
            pr_info("Calc Driver: Unknown operator\n"); return IRQ_HANDLED;
    }

    pr_info("Calc Driver: Result of %d %c %d = %d\n", my_data.x, my_data.op, my_data.y, result);
    
    // Optional: reset flag so it only calculates once per input
    my_data.data_ready = false; 

    return IRQ_HANDLED;
}


static ssize_t basic_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    char k_buf[32];

    if(*offset >= 32)
        return 0;

    int bytes = min(count, sizeof(k_buf) - 1);

    if (copy_from_user(k_buf, buf, bytes)) 
        return -EFAULT;

    k_buf[bytes] = '\0';

    // Parse the input: "10 + 20"
    if (sscanf(k_buf, "%d %c %d", &my_data.x, &my_data.op, &my_data.y) == 3) {
        my_data.data_ready = true;
        pr_info("Calc Driver: Data received! Press a key to calculate.\n");
    } else {
        pr_err("Calc Driver: Invalid format. Use 'num1 num2 op'\n");
    }

    return count;
}

struct file_operations fops ={
    .owner = THIS_MODULE,
    .write = basic_write,
};


static int __init irq_threaded_init(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) return major_number;

    // Use &major_number as a unique cookie for the shared IRQ
    int ret = request_threaded_irq(IRQ_NUM, irq_top, irq_thread, 
                                   IRQF_SHARED, "calc_irq", (void *)&major_number);
    if (ret) {
        pr_err("Calc Driver: Error requesting IRQ %d\n", IRQ_NUM);
        unregister_chrdev(major_number, DEVICE_NAME);
        return ret;
    }

    pr_info("Calc Driver Loaded. Major: %d. Press a key to trigger.\n", major_number);
    return 0;
}

static void __exit irq_threaded_exit(void)
{
    // Use the SAME cookie used in request_threaded_irq
    free_irq(IRQ_NUM, (void *)&major_number);
    unregister_chrdev(major_number, DEVICE_NAME);
    pr_info("Calc Driver Unloaded.\n");
}


module_init(irq_threaded_init);
module_exit(irq_threaded_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIDDARTH M");
MODULE_DESCRIPTION("Simple Linux Kernel Threaded Keyboard Interrupt Driver");