#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>
#include <linux/init.h>

static struct kobject demo_kobj;
static int demo_value,sum,sub,mul,div;
int num1,num2;
/* kobject type */
static void demo_release(struct kobject *kobj)
{
    printk(KERN_INFO "demo_kobj released\n");
}
static struct kobj_type demo_ktype = {
    .sysfs_ops = &kobj_sysfs_ops,.release=demo_release
};

char str[10];

/* Show */
static ssize_t value_show(struct kobject *kobj,
                         struct kobj_attribute *attr,
                         char *buf)
{
	printk(KERN_ALERT"In value show\n");
	if(strncmp(str,"sum",3)==0)
	{
		printk(KERN_INFO"Im sum\n");
		sum=num1+num2;
		int n=sprintf(buf,"sum:%d\n",sum);
		//printk(KERN_ALERT"%s\n",buf);
		return n; 
	}else if(strncmp(str,"sub",3)==0)
	{
		sub=num1-num2;
		return sprintf(buf, "sub:%d\n", sub);

	}else if(strncmp(str,"mul",3)==0)
	{
		mul=num1*num2;
		return sprintf(buf, "mul:%d\n", mul);

	}else if(strncmp(str,"div",3)==0)
	{
		if(num2==0)
			return -EFAULT;
		div=num1/num2;
		return sprintf(buf, "sum:%d\n", div);
	}
	else
		return sprintf(buf,"Invalid operation\n");;
}

/* Store */
static ssize_t value_store(struct kobject *kobj,
                          struct kobj_attribute *attr,
                          const char *buf, size_t count)
{
	printk(KERN_ALERT"In value store\n");
//    kstrtoint(buf, 10, &demo_value);
	sscanf(buf,"%d%d%s",&num1,&num2,str);
	printk("num1:%d num2:%d op:%s\n",num1,num2,str);
  	return count;
}

static struct kobj_attribute value_attr =
    __ATTR(value, 0664, value_show, value_store);

/* Init */
static int __init sysfs_demo_init(void)
{
    int ret;
printk(KERN_ALERT"In value init\n");
    ret = kobject_init_and_add(&demo_kobj, &demo_ktype,
                               kernel_kobj, "sysfs_demo");
    if (ret)
        return ret;

    ret = sysfs_create_file(&demo_kobj, &value_attr.attr);
    if (ret) {
        kobject_put(&demo_kobj);
        return ret;
    }

    pr_info("Sysfs_demo loaded\n");
    return 0;
}

/* Exit */
static void __exit sysfs_demo_exit(void)
{
    sysfs_remove_file(&demo_kobj, &value_attr.attr);
    kobject_put(&demo_kobj);
    pr_info("sysfs_demo Unloaded\n");
}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Simple sysfs driver");
MODULE_AUTHOR("Satheesh");
