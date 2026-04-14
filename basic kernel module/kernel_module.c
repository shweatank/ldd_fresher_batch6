#include <linux/module.h> //Required for all kernel modules
#include<linux/kernel.h>   //Required for printk
#include <linux/init.h>   //Required for __init and __exit macros

MODULE_LICENSE("GPL");
//MODULE_AUTHOR("Almas");
//MODULE_DESCRIPTION("Most basic linux kernel module for education");

/*
 * __init tells the kernel
 * "This function is only needed during module loading."
 * After successful load, the memory used by the function
 * can be freed.
 */

static int __init basic_module_init(void)
{
	printk(KERN_INFO "Basic kernel module loaded\n");
	return 0;  //Returning 0 means successful load
}

/*
 * __exit tells the kernel:
 * "This function is only needed during module removal"
 * It will NOT be included if the module is built into the kernel.
 */

static void __exit basic_module_exit(void)
{
	printk(KERN_INFO "Basic kernel module unloaded\n");
}

/*
 * These macros tell the kernel which functions
 * should be called when module is inserted and removed
 */

module_init(basic_module_init);
module_exit(basic_module_exit);


