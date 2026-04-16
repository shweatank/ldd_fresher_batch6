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
	{ 0x550b89f1, "kernel_kobj" },
	{ 0x83f1675b, "kobject_create_and_add" },
	{ 0x991982ac, "sysfs_create_file_ns" },
	{ 0xedc3f731, "kobject_put" },
	{ 0xe8213e80, "_printk" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0xd272d446, "__fentry__" },
	{ 0xdd6830c7, "sprintf" },
	{ 0xd09b06f5, "kstrtoint" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x550b89f1,
	0x83f1675b,
	0x991982ac,
	0xedc3f731,
	0xe8213e80,
	0xd272d446,
	0xd272d446,
	0xdd6830c7,
	0xd09b06f5,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"kernel_kobj\0"
	"kobject_create_and_add\0"
	"sysfs_create_file_ns\0"
	"kobject_put\0"
	"_printk\0"
	"__x86_return_thunk\0"
	"__fentry__\0"
	"sprintf\0"
	"kstrtoint\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "DE293F6C3C77B71996CF08F");
