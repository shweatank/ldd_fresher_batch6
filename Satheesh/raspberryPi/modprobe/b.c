#include <linux/module.h>
#include <linux/kernel.h>

extern void say_hello(void);

void say_world(void)
{
    say_hello();

    pr_info("Hello from DRIVER B\n");
}

EXPORT_SYMBOL(say_world);

static int __init b_init(void)
{
    pr_info("DRIVER B LOADED\n");
    return 0;
}

static void __exit b_exit(void)
{
    pr_info("DRIVER B REMOVED\n");
}

module_init(b_init);
module_exit(b_exit);

MODULE_LICENSE("GPL");
