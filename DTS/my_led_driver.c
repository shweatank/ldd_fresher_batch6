// my_led_driver.c
//blink led usind  DTS
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>
#include <linux/delay.h>

struct my_led_dev {
    struct gpio_desc *gpiod;
};

static int my_led_probe(struct platform_device *pdev)
{
    struct my_led_dev *led;
    int ret;

    pr_info("my_led: probe called\n");

    led = devm_kzalloc(&pdev->dev, sizeof(*led), GFP_KERNEL);
    if (!led)
        return -ENOMEM;

    // Get GPIO from DTS
    led->gpiod = devm_gpiod_get(&pdev->dev, NULL, GPIOD_OUT_LOW);
    if (IS_ERR(led->gpiod)) {
        pr_err("Failed to get GPIO\n");
        return PTR_ERR(led->gpiod);
    }

    platform_set_drvdata(pdev, led);

    // Simple blink (for demo)
    pr_info("my_led: blinking...\n");
    for (int i = 0; i < 10; i++) {
        gpiod_set_value(led->gpiod, 1);
        msleep(500);
        gpiod_set_value(led->gpiod, 0);
        msleep(500);
    }

    pr_info("my_led: probe done\n");

    return 0;
}

static void my_led_remove(struct platform_device *pdev)
{
    pr_info("my_led: remove called\n");
    return;
}

static const struct of_device_id my_led_of_match[] = {
    { .compatible = "prashant,my-led" },
    { }
};
MODULE_DEVICE_TABLE(of, my_led_of_match);

static struct platform_driver my_led_driver = {
    .probe  = my_led_probe,
    .remove = my_led_remove,
    .driver = {
        .name           = "my_led_driver",
        .of_match_table = my_led_of_match,
    },
};

module_platform_driver(my_led_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("Simple GPIO LED Driver (BCM2711)");
