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
	{ 0x67628f51, "msleep" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x02996a3c, "kthread_create_on_node" },
	{ 0xe48388e9, "wake_up_process" },
	{ 0x228b1f9c, "kthread_stop" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x5e505530, "kthread_should_stop" },
	{ 0x984622ae, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x67628f51,
	0xd272d446,
	0x02996a3c,
	0xe48388e9,
	0x228b1f9c,
	0xd272d446,
	0xe8213e80,
	0x5e505530,
	0x984622ae,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"msleep\0"
	"__x86_return_thunk\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"kthread_stop\0"
	"__fentry__\0"
	"_printk\0"
	"kthread_should_stop\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "6EA5B41FC839399EF152DA3");
