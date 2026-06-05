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
	{ 0x173ec8da, "sscanf" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0xd272d446, "schedule" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0x5403c125, "__init_waitqueue_head" },
	{ 0xb368f2a2, "__register_chrdev" },
	{ 0xfe5422fa, "hrtimer_setup" },
	{ 0x5fa07cc0, "hrtimer_start_range_ns" },
	{ 0x36a36ab1, "hrtimer_cancel" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0x16ab4215, "__wake_up" },
	{ 0x5a844b26, "__x86_indirect_thunk_rax" },
	{ 0x49fc4616, "hrtimer_forward" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x546c19d9, "validate_usercopy_range" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x984622ae, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x173ec8da,
	0xbd03ed67,
	0x7851be11,
	0x7a5ffe84,
	0x0db8d68d,
	0xd272d446,
	0xc87f4bab,
	0x092a35a2,
	0xd272d446,
	0x5403c125,
	0xb368f2a2,
	0xfe5422fa,
	0x5fa07cc0,
	0x36a36ab1,
	0x52b15b3b,
	0x16ab4215,
	0x5a844b26,
	0x49fc4616,
	0xd272d446,
	0xe8213e80,
	0xd272d446,
	0x546c19d9,
	0xa61fd7aa,
	0x092a35a2,
	0x984622ae,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"sscanf\0"
	"__ref_stack_chk_guard\0"
	"__SCT__might_resched\0"
	"init_wait_entry\0"
	"prepare_to_wait_event\0"
	"schedule\0"
	"finish_wait\0"
	"_copy_to_user\0"
	"__stack_chk_fail\0"
	"__init_waitqueue_head\0"
	"__register_chrdev\0"
	"hrtimer_setup\0"
	"hrtimer_start_range_ns\0"
	"hrtimer_cancel\0"
	"__unregister_chrdev\0"
	"__wake_up\0"
	"__x86_indirect_thunk_rax\0"
	"hrtimer_forward\0"
	"__fentry__\0"
	"_printk\0"
	"__x86_return_thunk\0"
	"validate_usercopy_range\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "469F069219E80FC4844D055");
