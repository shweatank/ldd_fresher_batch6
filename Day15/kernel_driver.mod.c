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
	{ 0x5e505530, "kthread_should_stop" },
	{ 0xd272d446, "__x86_return_thunk" },
	{ 0x7f79e79a, "kthread_create_on_node" },
	{ 0x630dad60, "wake_up_process" },
	{ 0x0571dc46, "kthread_stop" },
	{ 0xd272d446, "__fentry__" },
	{ 0xe8213e80, "_printk" },
	{ 0x67628f51, "msleep" },
	{ 0xbebe66ff, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0x5e505530,
	0xd272d446,
	0x7f79e79a,
	0x630dad60,
	0x0571dc46,
	0xd272d446,
	0xe8213e80,
	0x67628f51,
	0xbebe66ff,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"kthread_should_stop\0"
	"__x86_return_thunk\0"
	"kthread_create_on_node\0"
	"wake_up_process\0"
	"kthread_stop\0"
	"__fentry__\0"
	"_printk\0"
	"msleep\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "6EA5B41FC839399EF152DA3");
