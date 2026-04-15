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
	{ 0x9479a1e8, "strnlen" },
	{ 0x7851be11, "__SCT__might_resched" },
	{ 0x546c19d9, "validate_usercopy_range" },
	{ 0xa61fd7aa, "__check_object_size" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x7a5ffe84, "init_wait_entry" },
	{ 0xd272d446, "schedule" },
	{ 0x0db8d68d, "prepare_to_wait_event" },
	{ 0xc87f4bab, "finish_wait" },
	{ 0xf64ac983, "__copy_overflow" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xe54e0a6b, "__fortify_panic" },
	{ 0xc02ea1a6, "__register_chrdev" },
	{ 0xfe5422fa, "hrtimer_setup" },
	{ 0x5403c125, "__init_waitqueue_head" },
	{ 0x36a36ab1, "hrtimer_cancel" },
	{ 0x2d88a3ab, "flush_work" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x5fa07cc0, "hrtimer_start_range_ns" },
	{ 0xaef1f20d, "system_wq" },
	{ 0x49733ad6, "queue_work_on" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x90a48d82, "__ubsan_handle_out_of_bounds" },
	{ 0x16ab4215, "__wake_up" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xbd03ed67, "__ref_stack_chk_guard" },
	{ 0x4749ded2, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x9479a1e8,
	0x7851be11,
	0x546c19d9,
	0xa61fd7aa,
	0x092a35a2,
	0x7a5ffe84,
	0xd272d446,
	0x0db8d68d,
	0xc87f4bab,
	0xf64ac983,
	0xd272d446,
	0xe54e0a6b,
	0xc02ea1a6,
	0xfe5422fa,
	0x5403c125,
	0x36a36ab1,
	0x2d88a3ab,
	0x52b15b3b,
	0x092a35a2,
	0x5fa07cc0,
	0xaef1f20d,
	0x49733ad6,
	0xd272d446,
	0xe8213e80,
	0x90a48d82,
	0x16ab4215,
	0xd272d446,
	0xbd03ed67,
	0x4749ded2,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"strnlen\0"
	"__SCT__might_resched\0"
	"validate_usercopy_range\0"
	"__check_object_size\0"
	"_copy_to_user\0"
	"init_wait_entry\0"
	"schedule\0"
	"prepare_to_wait_event\0"
	"finish_wait\0"
	"__copy_overflow\0"
	"__stack_chk_fail\0"
	"__fortify_panic\0"
	"__register_chrdev\0"
	"hrtimer_setup\0"
	"__init_waitqueue_head\0"
	"hrtimer_cancel\0"
	"flush_work\0"
	"__unregister_chrdev\0"
	"_copy_from_user\0"
	"hrtimer_start_range_ns\0"
	"system_wq\0"
	"queue_work_on\0"
	"__fentry__\0"
	"_printk\0"
	"__ubsan_handle_out_of_bounds\0"
	"__wake_up\0"
	"__x86_return_thunk\0"
	"__ref_stack_chk_guard\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "D1C530951D47153227A1F2B");
