/*
  * it only demonstrates how code is loaded into
  * and removed from the linux kernel.
*/



#include<linux/module.h>//required for all kernel modules
#include<linux/kernel.h>//reqired for printk()
#include<linux/init.h>//required for __init nad __exit macros

MODULE_LICENSE("GPL"); //PREVENTS KERNEL TAINT
MODULE_AUTHOR("prashant");
MODULE_DESCRIPTION("Most basic linux kernel module for education");


/*
  init tells the kernel:
  "this function is only needed during module loading"
  After successful load,the memory used by this function
  can be freed.  
*/

static int __init basic_module_init(void)
{
        printk(KERN_INFO "Basic kernel module loaded\n");
        return 0;//returning 0 means suceessfully loaded
}

/*
   __exit tells the kernel:
   "this function isa only needed during module removal"
   it will not be included if the module is built into the kernel
*/
static void __exit basic_module_exit(void)
{
        printk(KERN_INFO "Basic kernel module unloaded\n");
}
/*
   these macros tell the kernel which functions
   should be called when is inserted
   and removed
*/

module_init(basic_module_init);
module_exit(basic_module_exit);


/*
   mandatory module metadata
   */

/*MODULE_LICENSE("GPL"); //PREVENTS KERNEL TAINT
MODULE_AUTHOR("prashant");
MODULE_DESCRIPTION("Most basic linux kernel module for education");*/
