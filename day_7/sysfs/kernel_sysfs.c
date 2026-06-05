#include <linux/module.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

static struct kobject *demo_kobj;
static int val1;
static int val2;
static int result;
static char op;

//show function

static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	switch(op){
		case '+':
			result=val1+val2;
			break;
		case '-':
			result=val1-val2;
			break;
		case '*':
			result=val1*val2;
	}
	//result=val1+val2;
	return sprintf(buf,"%d\n",result);
}

//store function

static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,
		const char *buf, size_t count)
{
	sscanf(buf, "%d,%d,%c",&val1,&val2,&op);
	return count;
}

static struct kobj_attribute value_attr=
	__ATTR(value, 0664,value_show, value_store);

static int __init sysfs_demo_init(void)
{
	demo_kobj = kobject_create_and_add("sysfs_demo",kernel_kobj);
	if(!demo_kobj)
		return -ENOMEM;

	sysfs_create_file(demo_kobj,&value_attr.attr);
	pr_info("sysfs_demo loaded\n");
	return 0;
}

static void __exit sysfs_demo_exit(void)
{
	kobject_put(demo_kobj);
	pr_info("sysfs_demo unloaded\n");

}

module_init(sysfs_demo_init);
module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("simple sysfs driver");
MODULE_AUTHOR("pavan");
