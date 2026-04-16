#include<linux/workqueue.h>
#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>
#include<linux/delay.h>
static struct work_struct my_work;

/*----------------work handler function------------------*/
static void my_work_handler(struct work_struct *work){
  pr_info("WorkQueue : Handler started\n");
  /*Simulate some work (sleep allowed)*/
  msleep(2000);
  pr_info("WorkQueue : Handler finished\n");
}
/*--------------------Module init-------------------*/
static int __init workq_init(void){
  pr_info("WorkQueue Module Loaded\n");

  /*Initialize Work*/
  INIT_WORK(&my_work,my_work_handler);

  /*schedule work*/
  pr_info("WorkQueue: scheduling work\n");
  schedule_work(&my_work);

  return 0;
}

/*-----------Module exit-----------------*/
static void __exit workq_exit(void){
  pr_info("WorkQueue Module Exiting\n");
  /*Ensure work is completed before exit*/
  flush_work(&my_work);
  pr_info("WorkQueue Module Unloaded\n");
}

module_init(workq_init);
module_exit(workq_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Rahul");
MODULE_DESCRIPTION("Simple WorkQueue Example for linux kernel 6.8\n");

