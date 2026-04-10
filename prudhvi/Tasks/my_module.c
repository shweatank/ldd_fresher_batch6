#include<linux/module.h>   //Requested for all kernel modules
#include<linux/kernel.h>  //Requested for printk()
#include<linux/init.h>    //Requested for __init and __exit macros
/*
->__init tells the kernel
     This function needed during module loading "
     After Successful load the memory used by this fnction
     can be freed.

*/
static int __init basic_module_init(void)
{
printk(KERN_INFO"Basic kernel module loaded\n");
return 0;//returing 0 means successful load
}
/*
->__exit tells the kernel
     this fuction is only needed during module removal
    It will no included if the mpodule is built into kernal  

*/

static void __exit basic_module_exit(void)
{
printk(KERN_INFO"Basic kernel module unloaded\n");
}
/*
->there macro tell the kernel which fuction 
->should be called when the module is inseted and removal

*/

module_init(basic_module_init);
module_exit(basic_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
MODULE_DESCRIPTION("Most basic Linux kernal module for education");
