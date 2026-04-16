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
	{ 0x16ab4215, "__wake_up" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0xd272d446, "schedule" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xa2a6a253, "remove_proc_entry" },
	{ 0x2d88a3ab, "flush_work" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0xaef1f20d, "system_wq" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xd272d446, "__fentry__" },
	{ 0x8cd5468a, "proc_create" },
	{ 0xe8213e80, "_printk" },
	{ 0x5403c125, "__init_waitqueue_head" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x4749ded2, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x16ab4215,
	0xbd03ed67,
	0x7851be11,
	0x7a5ffe84,
	0x0db8d68d,
	0xd272d446,
	0xc87f4bab,
	0x092a35a2,
	0xd272d446,
	0xa2a6a253,
	0x2d88a3ab,
	0x092a35a2,
	0xaef1f20d,
	0x49733ad6,
	0xd272d446,
	0x8cd5468a,
	0xe8213e80,
	0x5403c125,
	0xd272d446,
	0x4749ded2,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__wake_up\0"
	"__ref_stack_chk_guard\0"
	"__SCT__might_resched\0"
	"init_wait_entry\0"
	"prepare_to_wait_event\0"
	"schedule\0"
	"finish_wait\0"
	"_copy_to_user\0"
	"__stack_chk_fail\0"
	"remove_proc_entry\0"
	"flush_work\0"
	"_copy_from_user\0"
	"system_wq\0"
	"queue_work_on\0"
	"__fentry__\0"
	"proc_create\0"
	"_printk\0"
	"__init_waitqueue_head\0"
	"__x86_return_thunk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "837924F209217C8BBDBB79E");
