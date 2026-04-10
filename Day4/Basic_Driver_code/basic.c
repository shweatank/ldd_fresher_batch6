#include<linux/module.h> //requ for all kernal modules
#include<linux/kernel.h> // for printk
#include<linux/init.h>  // for __int and __exit

MODULE_LICENSE("GPL");
/*
 * __init tells the kernal: "this is only needed during module loading " after successful load , the memory
 * used by this function can be freed..
 * */
static int __init basic_module_init(void){
  printk(KERN_INFO "Basic kernal moduke loaded\n");
  return 0; //returned 0 means successfully loaded
}
/*
 * __exiit tells the kernal: "this is only needed during module removal " 
 * It will not be included if the module is built into kernel..
 * */
static void __exit  basic_module_exit(void){
  printk(KERN_INFO "Basic kernal moduke unloaded\n");
}
/*
 *These macros tell kernal 
 * */
module_init(basic_module_init);
module_exit(basic_module_exit);
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Thus is my first kernal module");
