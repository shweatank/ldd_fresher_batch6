/*
 * It only demonstrates how code is loaded into
 * and removed from the linux kernel.
 */

#include<linux/module.h>  //Required for all kernel modules
#include<linux/kernel.h> //required for printk()
#include<linux/init.h>   //required for __init and __exit macros

/*
 * __init tells the kernel:
 * "This function is only needed during module loading."
 * After succesful load,the memory used by this function
 * can be freed
 */

static int __init basic_module_init(void)
{
	printk(KERN_INFO "basic kernel module loaded\n");
	return 0;
}

/*
 * __exit tells the kernel:
 * "This function is only needed during module unloading."
 * It will NOT be included if the module is built into the kernel.
 */

static void __exit basic_module_exit(void)
{
	printk(KERN_INFO "basic kernel module unloaded\n");
}

/*
 * These macros tell the kernel which functions
 * should be called when the module is inserted
 * and removed.
 */

module_init(basic_module_init);
module_exit(basic_module_exit);

/*
 * Mandatory module metadata
 */

MODULE_LICENSE("GPL");  //prevents kernel taint
MODULE_AUTHOR("Sarika");
MODULE_DESCRIPTION("Most basic linux kernel module for education");
