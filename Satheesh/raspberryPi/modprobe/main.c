#include <linux/module.h>
#include <linux/kernel.h>

extern void say_world(void);

static int __init main_init(void)
{
    pr_info("MAIN DRIVER LOADED\n");

    say_world();

    return 0;
}

static void __exit main_exit(void)
{
    pr_info("MAIN DRIVER REMOVED\n");
}

module_init(main_init);
module_exit(main_exit);

MODULE_LICENSE("GPL");
