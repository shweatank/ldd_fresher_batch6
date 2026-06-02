// bcm2711_gpio_irq.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>

#define DRIVER_NAME "bcm2711_gpio_irq"

// BCM2711 GPIO base
#define GPIO_BASE_PHYS  0xFE200000
#define GPIO_SIZE       0xB4

// Register offsets
#define GPFSEL0   0x00
#define GPSET0    0x1C
#define GPCLR0    0x28
#define GPLEV0    0x34
#define GPEDS0    0x40
#define GPREN0    0x4C
#define GPFEN0    0x58

#define GPIO_PIN  17   // example GPIO17
#define GPIO_LED  18   //LED

int gpio_num= 17+512;

static void __iomem *gpio_base;
static int irq_number = 0; // must match DT / IRQ mapping
static struct work_struct my_work;

static void my_work_handler(struct work_struct *work)
{
        pr_info("Workqueue: Handler Started\n");
	
        //simulate some work (sleep allowed)
        writel((1 << GPIO_LED),gpio_base + GPSET0);
        msleep(2000);
        writel((0 << GPIO_LED),gpio_base + GPSET0);

        pr_info("Workqueue: Handler Finished\n");
}


// IRQ handler
static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    u32 status;

    status = readl(gpio_base + GPEDS0);
    if (status & (1 << GPIO_PIN)) {
        pr_info("GPIO %d interrupt triggered\n", GPIO_PIN);

        // Clear interrupt
	schedule_work(&my_work);
	writel((1 << GPIO_PIN),gpio_base + GPEDS0);

        return IRQ_HANDLED;
    }

    return IRQ_NONE;
}
//Configure GPIO for LED as Output
static void gpio_config_output(int pin)
{
        u32 val;
    int reg = pin / 10;
    int shift = (pin % 10) * 3;

    val = readl(gpio_base + GPFSEL0 + (reg * 4));
    val &= ~(7 << shift);  // input = 000
    val |=(1<<shift);
    writel(val, gpio_base + GPFSEL0 + (reg * 4));

}
// Configure GPIO as input
static void gpio_config_input(int pin)
{
    u32 val;
    int reg = pin / 10;
    int shift = (pin % 10) * 3;

    val = readl(gpio_base + GPFSEL0 + (reg * 4));
    val &= ~(7 << shift);  // input = 000
    writel(val, gpio_base + GPFSEL0 + (reg * 4));
}

// Enable rising edge interrupt
static void gpio_enable_irq(int pin)
{
    u32 val;

    val = readl(gpio_base + GPREN0);
    val |= (1 << pin);
    writel(val, gpio_base + GPREN0);
}

static int __init gpio_irq_init(void)
{
    pr_info(DRIVER_NAME ": init\n");

    gpio_base = ioremap(GPIO_BASE_PHYS, GPIO_SIZE);
    if (!gpio_base) {
        pr_err("Failed to map GPIO\n");
        return -ENOMEM;
    }
    
    INIT_WORK(&my_work, my_work_handler);
    gpio_config_input(GPIO_PIN);
    gpio_config_output(GPIO_LED);
    gpio_enable_irq(GPIO_PIN);

    //   Hardcoded IRQ (example only)
    //irq_number = 96; // depends on mapping
    irq_number = gpio_to_irq(gpio_num);
    if(irq_number < 0)
    {
	    pr_err("Failed to map GPIO to IRQ\n");
	    iounmap(gpio_base);
	    return irq_number;
    }

    writel(1 << GPIO_PIN, gpio_base + GPEDS0);
    if (request_irq(irq_number, gpio_irq_handler,
                    IRQF_SHARED, DRIVER_NAME, &irq_number)) {
        pr_err("Cannot register IRQ\n");
        iounmap(gpio_base);
        return -EIO;
    }

    pr_info("GPIO IRQ driver loaded\n");
    return 0;
}

static void __exit gpio_irq_exit(void)
{
    free_irq(irq_number, &irq_number);
    flush_work(&my_work);
    iounmap(gpio_base);
    pr_info(DRIVER_NAME ": exit\n");
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("techdhaba");
MODULE_DESCRIPTION("Minimal BCM2711 GPIO Interrupt Driver");
