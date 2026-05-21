#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/interrupt.h>
#include <linux/delay.h>

struct gpio_irq_led_dev {
    struct gpio_desc *irq_gpiod;
    struct gpio_desc *led_gpiod;
    int irq_number;
    atomic64_t count;
};

static irqreturn_t gpio_irq_handler(int irq, void *dev_id)
{
    struct gpio_irq_led_dev *data = dev_id;
    s64 c;

    c = atomic64_inc_return(&data->count);

    pr_info("IRQ Triggered! Count = %lld\n", c);

    /* Toggle LED */
    gpiod_set_value(data->led_gpiod,
        !gpiod_get_value(data->led_gpiod));

    return IRQ_HANDLED;
}

static int my_probe(struct platform_device *pdev)
{
    struct gpio_irq_led_dev *data;
    int ret;

    pr_info("my_gpio_irq_led: probe called\n");

    data = devm_kzalloc(&pdev->dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    atomic64_set(&data->count, 0);

    /* Get IRQ GPIO */
    data->irq_gpiod = devm_gpiod_get(&pdev->dev,
                                      "irq",
                                      GPIOD_IN);
    if (IS_ERR(data->irq_gpiod))
        return PTR_ERR(data->irq_gpiod);

    /* Get LED GPIO */
    data->led_gpiod = devm_gpiod_get(&pdev->dev,
                                      "led",
                                      GPIOD_OUT_LOW);
    if (IS_ERR(data->led_gpiod))
        return PTR_ERR(data->led_gpiod);

    /* Convert GPIO → IRQ */
    data->irq_number = gpiod_to_irq(data->irq_gpiod);
    if (data->irq_number < 0)
        return data->irq_number;

    pr_info("Mapped IRQ = %d\n", data->irq_number);

    /* Request IRQ */
    ret = devm_request_threaded_irq(
        &pdev->dev,
        data->irq_number,
        NULL,
        gpio_irq_handler,
        IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
        "gpio_irq_led",
        data
    );

    if (ret) {
        pr_err("Failed to request IRQ\n");
        return ret;
    }

    platform_set_drvdata(pdev, data);

    pr_info("Driver loaded successfully\n");
    return 0;
}

static void my_remove(struct platform_device *pdev)
{
    pr_info("my_gpio_irq_led: removed\n");
}

/* Device Tree Match */
static const struct of_device_id my_dt_ids[] = {
    { .compatible = "prashant,my-gpio-irq-led" },
    { }
};
MODULE_DEVICE_TABLE(of, my_dt_ids);

static struct platform_driver my_driver = {
    .probe  = my_probe,
    .remove = my_remove,
    .driver = {
        .name = "my_gpio_irq_led",
        .of_match_table = my_dt_ids,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("GPIO IRQ + LED Driver using DTS");
