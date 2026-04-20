/*
driver :
write into procfs using user program and after receiving data ....
The calculation work giving to the workqueue...
Reader waits till the workqueue completes its task and allows  after that to send data to user process. */

#include<linux/module.h>
#include<linux/proc_fs.h>
#include<linux/uaccess.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/init.h>

#include<linux/timer.h>
#include<linux/jiffies.h>

#include<linux/workqueue.h>

#include<linux/wait.h>


#define PROC_NAME "proc_calculator_workQ"
#define BUF_SIZE 128
static int val=0;
static struct work_struct my_work;
static wait_queue_head_t wq;
static int work_done=0;
static char proc_buffer[BUF_SIZE];

/*work handler function */
static void my_work_handler(struct work_struct *work)
{
        int first=0,second=0,i=0;
        pr_info("Workqueue: Handler started\n");

         while(proc_buffer[i])
     {
         if(proc_buffer[i]==',')
         {
             i++;
             while(proc_buffer[i]!=',')
             {
                  second=second*10+(proc_buffer[i]-'0');
              i++;
             }
             i++;
             break;
         }
         first=first*10+(proc_buffer[i]-'0');
         i++;
     }

     if(proc_buffer[i]=='A')
     {
         val=first+second;

     }
     else if(proc_buffer[i]=='S')
     {
         val=first-second;

     }
     else if(proc_buffer[i]=='M')
     {
         val=first*second;
     }
     else
     {
         printk(KERN_INFO "invalid operation given\n");
     }

     printk(KERN_INFO "value=%d\n",val);
     sprintf(proc_buffer,"%d",val);


        
        pr_info("Workqueue: Handler finished\n");

        work_done=1;
        wake_up_interruptible(&wq);
}



/* Read callback */

static ssize_t proc_read(struct file *file,char __user *user_buf,
                size_t count,loff_t *ppos)
{
        wait_event_interruptible(wq,work_done!=0);
        pr_info("Inside proc read func \n");
        count=min(count,BUF_SIZE);

        if(copy_to_user(user_buf,proc_buffer,count))                
        {
                return -EFAULT;
        }

        pr_info("Read: done\n");
        work_done=0;

        (*ppos)=0;

        return count;
}

/* Write callback */
static ssize_t proc_write(struct file *file,const char __user *user_buf,
                size_t count,loff_t *ppos)
{
        pr_info("Inside proc write func \n");
        if(count > BUF_SIZE-1)
                return -EFAULT;

        if(copy_from_user(proc_buffer,user_buf,count))
                return -EFAULT;

        proc_buffer[count]='\0';

        schedule_work(&my_work);

        return count;
}


static const struct proc_ops proc_file_ops={
        .proc_read=proc_read,
        .proc_write=proc_write,
};

static int __init proc_demo_init(void)
{
        proc_create(PROC_NAME,0666,NULL,&proc_file_ops);
        pr_info("proc_demo loaded\n");


         pr_info("workqueue module loaded\n");
        /* Initalize work */
        INIT_WORK(&my_work,my_work_handler);


        printk(KERN_INFO "waitqueue initialization done\n");
        init_waitqueue_head(&wq);

        return 0;
}

static void __exit proc_demo_exit(void)
{
        remove_proc_entry(PROC_NAME,NULL);
        pr_info("proc_demo unloaded\n");


         pr_info("Workqueue module exiting\n");

        /* Ensure work is completed before exit */
        flush_work(&my_work);

        pr_info("Workqueue module unloaded\n");
};


module_init(proc_demo_init);
module_exit(proc_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple procfs driver");
MODULE_AUTHOR("TechDhaba");
