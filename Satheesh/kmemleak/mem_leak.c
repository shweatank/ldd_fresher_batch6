#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charan");
MODULE_DESCRIPTION("Kernel Memory Leak Example for kmemleak");

static char *ptr;

static int __init mem_leak_init(void)
{
    printk(KERN_INFO "mem_leak: Module loading\n");

    ptr = kmalloc(1024, GFP_KERNEL);
    if (!ptr) {
        printk(KERN_ERR "mem_leak: kmalloc failed\n");
        return -ENOMEM;
    }

    printk(KERN_INFO "mem_leak: Allocated 1024 bytes at %px\n", ptr);

    /* Intentionally not freeing ptr to create a memory leak */
    return 0;
}

static void __exit mem_leak_exit(void)
{
    printk(KERN_INFO "mem_leak: Module unloading\n");

    /* Intentionally not calling kfree(ptr) */
    printk(KERN_INFO "mem_leak: Memory intentionally leaked\n");
}

module_init(mem_leak_init);
module_exit(mem_leak_exit);
