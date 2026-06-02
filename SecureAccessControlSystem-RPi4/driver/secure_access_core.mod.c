#include <linux/module.h>
#include <linux/export-internal.h>
#include <linux/compiler.h>

MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0x9f222e1e, "alloc_chrdev_region" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0xd5ad82a1, "misc_deregister" },
	{ 0x075b6205, "devm_request_threaded_irq" },
	{ 0x0040afbe, "param_ops_uint" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x79fd4f39, "devm_kmalloc" },
	{ 0x4695bf9b, "platform_driver_unregister" },
	{ 0x40a621c5, "snprintf" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0xd2e1e487, "gpiod_set_value_cansleep" },
	{ 0xa1dacb42, "class_destroy" },
	{ 0x40a621c5, "scnprintf" },
	{ 0x4073d0de, "up" },
	{ 0x12ad300e, "iounmap" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0x8d6a2b09, "devm_gpiod_get_optional" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0x16ab4215, "__wake_up" },
	{ 0xe1e1f979, "_raw_spin_lock_irqsave" },
	{ 0xbe011736, "__dynamic_dev_dbg" },
	{ 0xd272d446, "__fentry__" },
	{ 0x630dad60, "wake_up_process" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0xe8213e80, "_printk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0xd272d446, "schedule" },
	{ 0x6ac784f4, "schedule_timeout" },
	{ 0x5a8347fe, "__tracepoint_sched_set_state_tp" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x7d74049b, "put_device" },
	{ 0x9b1de7cb, "_dev_info" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x8ea73856, "cdev_add" },
	{ 0x678eaed8, "device_create_file" },
	{ 0x9c0551c6, "tasklet_kill" },
	{ 0x33318883, "spi_sync" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0xe486c4b7, "device_create" },
	{ 0xcdec1689, "tasklet_init" },
	{ 0x23ef80fb, "noop_llseek" },
	{ 0x653aa194, "class_create" },
	{ 0xf46d5bf3, "mutex_lock" },
	{ 0x2719b9fa, "const_current_task" },
	{ 0x9c0551c6, "__tasklet_schedule" },
	{ 0x97dd6ca9, "ioremap" },
	{ 0x0571dc46, "kthread_stop" },
	{ 0xc1e6c71e, "__mutex_init" },
	{ 0x81a1a811, "_raw_spin_unlock_irqrestore" },
	{ 0x9b1de7cb, "_dev_warn" },
	{ 0xaca12394, "misc_register" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x5403c125, "__init_waitqueue_head" },
	{ 0x888b8f57, "strcmp" },
	{ 0x223cc85c, "__platform_driver_register" },
	{ 0x7f79e79a, "kthread_create_on_node" },
	{ 0x0bc5fb0d, "unregister_chrdev_region" },
	{ 0xf46d5bf3, "mutex_unlock" },
	{ 0xb689121e, "strnstr" },
	{ 0x1595e410, "device_destroy" },
	{ 0x2d88a3ab, "cancel_work_sync" },
	{ 0x546c19d9, "validate_usercopy_range" },
	{ 0x12694399, "down_interruptible" },
	{ 0x96e1c7bf, "platform_get_irq_optional" },
	{ 0xe4de56b4, "__ubsan_handle_load_invalid_value" },
	{ 0xf1de9e85, "kvfree" },
	{ 0x437e81c7, "simple_read_from_buffer" },
	{ 0xf52f8b44, "__kvmalloc_node_noprof" },
	{ 0x67628f51, "msleep" },
	{ 0xd5f66efd, "cdev_init" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0xb2e62cba, "__trace_set_current_state" },
	{ 0x4e54d6ac, "cdev_del" },
	{ 0xd4f60fb2, "device_remove_file" },
	{ 0xaef1f20d, "system_wq" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x9f222e1e,
	0xa61fd7aa,
	0xd5ad82a1,
	0x075b6205,
	0x0040afbe,
	0x092a35a2,
	0x79fd4f39,
	0x4695bf9b,
	0x40a621c5,
	0x49733ad6,
	0xc87f4bab,
	0xd2e1e487,
	0xa1dacb42,
	0x40a621c5,
	0x4073d0de,
	0x12ad300e,
	0x0db8d68d,
	0x8d6a2b09,
	0x5e505530,
	0x16ab4215,
	0xe1e1f979,
	0xbe011736,
	0xd272d446,
	0x630dad60,
	0x5a844b26,
	0xe8213e80,
	0xbd03ed67,
	0xd272d446,
	0x6ac784f4,
	0x5a8347fe,
	0xd272d446,
	0x7d74049b,
	0x9b1de7cb,
	0x90a48d82,
	0x8ea73856,
	0x678eaed8,
	0x9c0551c6,
	0x33318883,
	0x7a5ffe84,
	0xe486c4b7,
	0xcdec1689,
	0x23ef80fb,
	0x653aa194,
	0xf46d5bf3,
	0x2719b9fa,
	0x9c0551c6,
	0x97dd6ca9,
	0x0571dc46,
	0xc1e6c71e,
	0x81a1a811,
	0x9b1de7cb,
	0xaca12394,
	0xd272d446,
	0x092a35a2,
	0x5403c125,
	0x888b8f57,
	0x223cc85c,
	0x7f79e79a,
	0x0bc5fb0d,
	0xf46d5bf3,
	0xb689121e,
	0x1595e410,
	0x2d88a3ab,
	0x546c19d9,
	0x12694399,
	0x96e1c7bf,
	0xe4de56b4,
	0xf1de9e85,
	0x437e81c7,
	0xf52f8b44,
	0x67628f51,
	0xd5f66efd,
	0x7851be11,
	0xb2e62cba,
	0x4e54d6ac,
	0xd4f60fb2,
	0xaef1f20d,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"alloc_chrdev_region\0"
	"__check_object_size\0"
	"misc_deregister\0"
	"devm_request_threaded_irq\0"
	"param_ops_uint\0"
	"_copy_from_user\0"
	"devm_kmalloc\0"
	"platform_driver_unregister\0"
	"snprintf\0"
	"queue_work_on\0"
	"finish_wait\0"
	"gpiod_set_value_cansleep\0"
	"class_destroy\0"
	"scnprintf\0"
	"up\0"
	"iounmap\0"
	"prepare_to_wait_event\0"
	"devm_gpiod_get_optional\0"
	"kthread_should_stop\0"
	"__wake_up\0"
	"_raw_spin_lock_irqsave\0"
	"__dynamic_dev_dbg\0"
	"__fentry__\0"
	"wake_up_process\0"
	"__x86_indirect_thunk_rax\0"
	"_printk\0"
	"__ref_stack_chk_guard\0"
	"schedule\0"
	"schedule_timeout\0"
	"__tracepoint_sched_set_state_tp\0"
	"__stack_chk_fail\0"
	"put_device\0"
	"_dev_info\0"
	"__ubsan_handle_out_of_bounds\0"
	"cdev_add\0"
	"device_create_file\0"
	"tasklet_kill\0"
	"spi_sync\0"
	"init_wait_entry\0"
	"device_create\0"
	"tasklet_init\0"
	"noop_llseek\0"
	"class_create\0"
	"mutex_lock\0"
	"const_current_task\0"
	"__tasklet_schedule\0"
	"ioremap\0"
	"kthread_stop\0"
	"__mutex_init\0"
	"_raw_spin_unlock_irqrestore\0"
	"_dev_warn\0"
	"misc_register\0"
	"__x86_return_thunk\0"
	"_copy_to_user\0"
	"__init_waitqueue_head\0"
	"strcmp\0"
	"__platform_driver_register\0"
	"kthread_create_on_node\0"
	"unregister_chrdev_region\0"
	"mutex_unlock\0"
	"strnstr\0"
	"device_destroy\0"
	"cancel_work_sync\0"
	"validate_usercopy_range\0"
	"down_interruptible\0"
	"platform_get_irq_optional\0"
	"__ubsan_handle_load_invalid_value\0"
	"kvfree\0"
	"simple_read_from_buffer\0"
	"__kvmalloc_node_noprof\0"
	"msleep\0"
	"cdev_init\0"
	"__SCT__might_resched\0"
	"__trace_set_current_state\0"
	"cdev_del\0"
	"device_remove_file\0"
	"system_wq\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");

MODULE_ALIAS("of:N*T*Csecure,sac-ctrl");
MODULE_ALIAS("of:N*T*Csecure,sac-ctrlC*");

MODULE_INFO(srcversion, "89E951802CF9244CBCD468E");
