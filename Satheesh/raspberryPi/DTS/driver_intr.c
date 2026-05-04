// my_gpio_driver.c

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>

struct my_gpio_dev {
    struct gpio_desc *button;
    struct gpio_desc *led;
    int irq;
};

static irqreturn_t button_isr(int irq, void *dev_id)
{
    struct my_gpio_dev *dev = dev_id;

    // Toggle LED
    int val = gpiod_get_value(dev->led);
    gpiod_set_value(dev->led, !val);

    pr_info("Button pressed! LED toggled\n");

    return IRQ_HANDLED;
}

static int my_probe(struct platform_device *pdev)
{
    struct my_gpio_dev *dev;
    int ret;

    dev = devm_kzalloc(&pdev->dev, sizeof(*dev), GFP_KERNEL);
    if (!dev)
        return -ENOMEM;

    // Get button GPIO
    dev->button = devm_gpiod_get(&pdev->dev, "button", GPIOD_IN);
    if (IS_ERR(dev->button)) {
        dev_err(&pdev->dev, "Failed to get button GPIO\n");
        return PTR_ERR(dev->button);
    }

    // Get LED GPIO
    dev->led = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(dev->led)) {
        dev_err(&pdev->dev, "Failed to get LED GPIO\n");
        return PTR_ERR(dev->led);
    }

    // Get IRQ from device tree
    dev->irq = platform_get_irq(pdev, 0);
    if (dev->irq < 0) {
        dev_err(&pdev->dev, "Failed to get IRQ\n");
        return dev->irq;
    }

    // Request interrupt
    ret = devm_request_irq(&pdev->dev, dev->irq,
                           button_isr,
                           IRQF_TRIGGER_FALLING,
                           "my_gpio_irq",
                           dev);
    if (ret) {
        dev_err(&pdev->dev, "Failed to request IRQ\n");
        return ret;
    }

    platform_set_drvdata(pdev, dev);

    dev_info(&pdev->dev, "My GPIO driver loaded\n");
    return 0;
}

/*static int my_remove(struct platform_device *pdev)
{
    dev_info(&pdev->dev, "My GPIO driver removed\n");
    return 0;
}*/


static const struct of_device_id my_of_match[] = {
    { .compatible = "mycompany,my-gpio-driver" },
    { }
};
MODULE_DEVICE_TABLE(of, my_of_match);

static struct platform_driver my_driver = {
    .probe  = my_probe,
//    .remove = my_remove,
    .driver = {
        .name = "my_gpio_driver",
        .of_match_table = my_of_match,
    },
};

module_platform_driver(my_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("You");
MODULE_DESCRIPTION("GPIO Interrupt + LED Driver");
