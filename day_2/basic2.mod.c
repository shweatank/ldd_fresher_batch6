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
	{ 0x173ec8da, "sscanf" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0xf64ac983, "__copy_overflow" },
	{ 0xd272d446, "__stack_chk_fail" },
	{ 0xb368f2a2, "__register_chrdev" },
	{ 0x52b15b3b, "__unregister_chrdev" },
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
	0xbd03ed67,
	0x173ec8da,
	0xdd6830c7,
	0x092a35a2,
	0xf64ac983,
	0xd272d446,
	0xb368f2a2,
	0x52b15b3b,
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
	"__ref_stack_chk_guard\0"
	"sscanf\0"
	"sprintf\0"
	"_copy_to_user\0"
	"__copy_overflow\0"
	"__stack_chk_fail\0"
	"__register_chrdev\0"
	"__unregister_chrdev\0"
	"__fentry__\0"
	"_printk\0"
	"__x86_return_thunk\0"
	"validate_usercopy_range\0"
	"__check_object_size\0"
	"_copy_from_user\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "956C2F83C44C7C9A862A235");
