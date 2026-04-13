#include<linux/module.h> //required for all kernel modules
#include<linux/kernel.h>//required for printk()
#include<linux/init.h>//required for __init and __exit macros
/* __init tells the kernal:
 * "This function is only needed during module loading."
 * After successful load,the memory used by this function
 * can be freed
 */
static int __init basic_module_init(void)
{
	printk(KERN_INFO"Basic kernel module loaded\n");
	return 0;
}
/*__exit tells the kernel
 * "this function is only needed during module removal"
 * it will not be included if the module is built into the kernel
 */
static void __exit basic_module_exit(void)
{
	printk(KERN_INFO"Basic kernel module unlocked\n");
}
/*these macros tell the kernel which functions 
 * should be called when the module is inserted
 * and removed
 */
module_init(basic_module_init);
module_exit(basic_module_exit);
/*mandatory module metadata*/
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prashant");
MODULE_DESCRIPTION("Most basic Linux Kernel module for education");
