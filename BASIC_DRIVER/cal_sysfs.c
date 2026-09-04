#include<linux/module.h>
#include<linux/kobject.h>
#include<linux/sysfs.h>

static struct kobject *demo_kobj;
static int num1,num2;
static char op;
static int res;
static ssize_t value_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
	switch(op)
	{
		case '+':
			res = num1+num2;
			break;
		case '-':
			res = num1-num2;
			break;
		case '*':
			res = num1*num2;
			break;
		case '/':
			if(num2 == 0)
				return -ENOMEM;
			res = num1/num2;
			break;
		default:
			pr_info("Invalid option\n");
			return -ENOMEM;
	}

	return sprintf(buf,"%d\n",res);
}
static ssize_t value_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
	sscanf(buf,"%d %c %d",&num1,&op,&num2);
	return count;
}

static struct kobj_attribute value_attr = __ATTR(calculator,0664,value_show,value_store);

static int __init sysfs_demo_init(void)
{
	demo_kobj = kobject_create_and_add("sysfs_cal",kernel_kobj);
	if(!demo_kobj)
		return -ENOMEM;

	sysfs_create_file(demo_kobj,&value_attr.attr);
	pr_info("sysfs_cal loaded\n");
	return 0;
}

static void __exit sysfs_demo_exit(void)
{
	kobject_put(demo_kobj);
	pr_info("sysfs_add unloaded\n");
}

module_init(sysfs_demo_init);

module_exit(sysfs_demo_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("sysfs create");

