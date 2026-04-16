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
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0xd272d446, "schedule" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x9c0551c6, "__tasklet_schedule" },
	{ 0x9126ce86, "request_threaded_irq" },
	{ 0x5403c125, "__init_waitqueue_head" },
	{ 0x94e2e501, "kthread_create_on_node" },
	{ 0x19b60d40, "wake_up_process" },
	{ 0x9c0551c6, "tasklet_kill" },
	{ 0x9dd4105e, "free_irq" },
	{ 0x89d69aa8, "kthread_stop" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x16ab4215, "__wake_up" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x4749ded2, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xbd03ed67,
	0x5e505530,
	0x7851be11,
	0x7a5ffe84,
	0xd272d446,
	0x0db8d68d,
	0xc87f4bab,
	0xd272d446,
	0x9c0551c6,
	0x9126ce86,
	0x5403c125,
	0x94e2e501,
	0x19b60d40,
	0x9c0551c6,
	0x9dd4105e,
	0x89d69aa8,
	0xd272d446,
	0xe8213e80,
	0x16ab4215,
	0xd272d446,
	0x4749ded2,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__ref_stack_chk_guard\0"
	"kthread_should_stop\0"
	"__SCT__might_resched\0"
	"init_wait_entry\0"
	"schedule\0"
	"prepare_to_wait_event\0"
	"finish_wait\0"
	"__stack_chk_fail\0"
	"__tasklet_schedule\0"
	"request_threaded_irq\0"
	"__init_waitqueue_head\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"tasklet_kill\0"
	"free_irq\0"
	"kthread_stop\0"
	"__fentry__\0"
	"_printk\0"
	"__wake_up\0"
	"__x86_return_thunk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "0325EFA0B23C654A6742F0C");
