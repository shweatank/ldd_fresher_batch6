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
	{ 0xd306d8ff, "kernel_kobj" },
	{ 0x8d188e46, "kobject_create_and_add" },
	{ 0xcc3efb9a, "sysfs_create_file_ns" },
	{ 0xe8213e80, "_printk" },
	{ 0xeae2c62b, "kobject_put" },
	{ 0xd272d446, "__fentry__" },
	{ 0x173ec8da, "sscanf" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xdd6830c7, "sprintf" },
	{ 0x984622ae, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd306d8ff,
	0x8d188e46,
	0xcc3efb9a,
	0xe8213e80,
	0xeae2c62b,
	0xd272d446,
	0x173ec8da,
	0xd272d446,
	0xdd6830c7,
	0x984622ae,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"kernel_kobj\0"
	"kobject_create_and_add\0"
	"sysfs_create_file_ns\0"
	"_printk\0"
	"kobject_put\0"
	"__fentry__\0"
	"sscanf\0"
	"__x86_return_thunk\0"
	"sprintf\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "00E02F70F12C101598A289A");
