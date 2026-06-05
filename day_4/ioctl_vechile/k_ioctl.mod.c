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
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xb368f2a2, "__register_chrdev" },
	{ 0xe8213e80, "_printk" },
	{ 0x52b15b3b, "__unregister_chrdev" },
	{ 0xd272d446, "__fentry__" },
	{ 0x092a35a2, "_copy_from_user" },
	{ 0x092a35a2, "_copy_to_user" },
	{ 0x984622ae, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0xb368f2a2,
	0xe8213e80,
	0x52b15b3b,
	0xd272d446,
	0x092a35a2,
	0x092a35a2,
	0x984622ae,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__x86_return_thunk\0"
	"__register_chrdev\0"
	"_printk\0"
	"__unregister_chrdev\0"
	"__fentry__\0"
	"_copy_from_user\0"
	"_copy_to_user\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "48ABE8B84540C2A31535E6F");
