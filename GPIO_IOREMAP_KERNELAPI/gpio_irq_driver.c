/*  25 pin when we short it to ground, it will be rising edge(0-1). When we remove the grnd connection it will be falling edge (1-0). THis acts as interrupt */
/* 512 is the offset */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

#define GPIO_IRQ_PIN   25+512   /* Input pin */
#define GPIO_LED_PIN   18+512   /* Output pin */

static int irq_number;
static atomic64_t irq_count = ATOMIC64_INIT(0);

/* Interrupt handler (threaded) */
static irqreturn_t gpio_irq_thread(int irq, void *dev_id)
{
    s64 count;

    count = atomic64_inc_return(&irq_count);

    pr_info("GPIO IRQ Triggered! Count = %lld\n", count);

    /* Toggle LED */
    gpio_set_value(GPIO_LED_PIN, !gpio_get_value(GPIO_LED_PIN));

    return IRQ_HANDLED;
}

static int __init gpio_irq_init(void)
{
    int ret;

    pr_info("GPIO IRQ Module Init\n");

    /* 1. Request IRQ GPIO */
    ret = gpio_request(GPIO_IRQ_PIN, "gpio_irq_pin");
    if (ret) {
        pr_err("Failed to request GPIO %d\n", GPIO_IRQ_PIN);
        return ret;
    }

    ret = gpio_direction_input(GPIO_IRQ_PIN);
    if (ret) {
        pr_err("Failed to set GPIO %d as input\n", GPIO_IRQ_PIN);
        goto err_free_irq_gpio;
    }

    /* 2. Request LED GPIO */
    ret = gpio_request(GPIO_LED_PIN, "gpio_led_pin");
    if (ret) {
        pr_err("Failed to request GPIO %d\n", GPIO_LED_PIN);
        goto err_free_irq_gpio;
    }

    ret = gpio_direction_output(GPIO_LED_PIN, 0);
    if (ret) {
        pr_err("Failed to set GPIO %d as output\n", GPIO_LED_PIN);
        goto err_free_led_gpio;
    }

    /* 3. Convert GPIO to IRQ */
    irq_number = gpio_to_irq(GPIO_IRQ_PIN);
    if (irq_number < 0) {
        pr_err("Failed to map GPIO %d to IRQ\n", GPIO_IRQ_PIN);
        ret = irq_number;
        goto err_free_led_gpio;
    }

    pr_info("GPIO %d mapped to IRQ %d\n", GPIO_IRQ_PIN, irq_number);

    /* 4. Request IRQ (FALLING EDGE) */
    ret = request_threaded_irq(
        irq_number,
        NULL,                     /* no hard IRQ handler */
        gpio_irq_thread,          /* threaded handler */
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
        "gpio_irq_demo",
        NULL
    );

    if (ret) {
        pr_err("Failed to request IRQ %d\n", irq_number);
        goto err_free_led_gpio;
    }

    pr_info("GPIO IRQ Driver Loaded Successfully\n");
    return 0;

err_free_led_gpio:
    gpio_free(GPIO_LED_PIN);

err_free_irq_gpio:
    gpio_free(GPIO_IRQ_PIN);

    return ret;
}

static void __exit gpio_irq_exit(void)
{
    free_irq(irq_number, NULL);
    gpio_free(GPIO_IRQ_PIN);
    gpio_free(GPIO_LED_PIN);

    pr_info("GPIO IRQ Module Unloaded\n");
}

module_init(gpio_irq_init);
module_exit(gpio_irq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("TechDhaba");
MODULE_DESCRIPTION("BCM2711 GPIO Interrupt Driver WITHOUT DTS (Kernel 6.8)");
