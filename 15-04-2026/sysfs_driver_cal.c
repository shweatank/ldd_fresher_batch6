#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>

static struct kobject *demo_kobj;
static int value1,value2;

/* store function */

static ssize_t value2_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
        sscanf(buf,"%d",&value2);
        return count;
}

/* Store function */

static ssize_t value1_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
        sscanf(buf,"%d",&value1);
        return count;
}

static ssize_t value2_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        return sprintf(buf,"%d\n",value2);
}

/* Store function */

static ssize_t value1_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        return sprintf(buf,"%d\n",value1);
}


/* Sum function */
static ssize_t sum_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
        int sum=value1+value2;
        return sprintf(buf,"%d",sum);
}



static struct kobj_attribute value1_attr =
    __ATTR(value1,0664,value1_show,value1_store);

static struct kobj_attribute value2_attr =
    __ATTR(value2,0664,value2_show,value2_store);

static struct kobj_attribute sum_attr =
    __ATTR(sum,0444,sum_show,NULL);


static int __init sysfs_demo_init(void)
{
        demo_kobj = kobject_create_and_add("sysfs_driver_cal",kernel_kobj);
        if(!demo_kobj)
                return -ENOMEM;
        int ret;

       ret=sysfs_create_file(demo_kobj,&value1_attr.attr);
       if(ret)
       {
               return ret;
       }
        ret=sysfs_create_file(demo_kobj,&value2_attr.attr);
        if(ret)
        {
                return ret;
        }
        ret=sysfs_create_file(demo_kobj,&sum_attr.attr);
        if(ret)
        {
                return ret;
        }
        pr_info("sysfs_driver_cal loaded\n");
        return 0;
}

static void __exit sysfs_demo_exit(void)
{
        sysfs_remove_file(demo_kobj,&value1_attr.attr);
        sysfs_remove_file(demo_kobj,&value2_attr.attr);
        sysfs_remove_file(demo_kobj,&sum_attr.attr);

        kobject_put(demo_kobj);
        pr_info("sysfs_driver_cal unloaded\n");

}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver sum calculator");
MODULE_AUTHOR("Techdhaba");
