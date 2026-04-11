/* It only demonstrates how code is loaded into 
 * and removed from the Linux Kernel.
 */

#include<linux/module.h> //req for all kernel modules
#include<linux/kernel.h> //req for printk()
#include<linux/init.h> //req for _init and _exit macros

/* _init tells the kernel:
 *  This function is only needed during module loading. "
 *  After successful load, the mem used by this function
 *  can be freed
 */

static int __init basic_module_init(void)
{
	printk(KERN_INFO "Basic kernel module loaded\n");
	return 0; //returning 0 means successful load
}

/*
 * __exit tells the kernel:
 * "This function is only needed during module removal."
 * It will NOT be included if the module is built into the kernel.
 */
static void __exit basic_module_exit(void)
{
	printk(KERN_INFO "Basic kernel module unloaded\n");
}

/*These macros tell the kernel which functions
 * should be called when the module is inserted
 * and removed
 */
module_init(basic_module_init);
module_exit(basic_module_exit);

/*
 * Mandatory module metadata
 */ 
MODULE_LICENSE("GPL");  //prevents kernel taint
MODULE_AUTHOR("Nandini");    
MODULE_DESCRIPTION("Most basic Linux kernel module for education");
