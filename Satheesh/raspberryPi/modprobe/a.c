#include <linux/module.h>
#include <linux/kernel.h>

void say_hello(void)
{
    pr_info("Hello from DRIVER A\n");
}

EXPORT_SYMBOL(say_hello);

static int __init a_init(void)
{
    pr_info("DRIVER A LOADED\n");
    return 0;
}

static void __exit a_exit(void)
{
    pr_info("DRIVER A REMOVED\n");
}

module_init(a_init);
module_exit(a_exit);

MODULE_LICENSE("GPL");
