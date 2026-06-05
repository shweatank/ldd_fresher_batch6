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
	{ 0xd272d446, "__fentry__" },
	{ 0x17cbfa50, "gpio_to_desc" },
	{ 0x1a18a627, "gpiod_get_raw_value" },
	{ 0x6dbf033c, "gpiod_set_raw_value" },
	{ 0x76a97ac1, "gpio_request" },
	{ 0xa095dbe6, "gpiod_direction_input" },
	{ 0x6dbf033c, "gpiod_direction_output_raw" },
	{ 0x1a18a627, "gpiod_to_irq" },
	{ 0x9126ce86, "request_threaded_irq" },
	{ 0x9dd4105e, "free_irq" },
	{ 0xebe5da4a, "gpio_free" },
	{ 0xe8213e80, "_printk" },
	{ 0x984622ae, "module_layout" },
};

static const u32 ____version_ext_crcs[]
__used __section("__version_ext_crcs") = {
	0xd272d446,
	0xd272d446,
	0x17cbfa50,
	0x1a18a627,
	0x6dbf033c,
	0x76a97ac1,
	0xa095dbe6,
	0x6dbf033c,
	0x1a18a627,
	0x9126ce86,
	0x9dd4105e,
	0xebe5da4a,
	0xe8213e80,
	0x984622ae,
};
static const char ____version_ext_names[]
__used __section("__version_ext_names") =
	"__x86_return_thunk\0"
	"__fentry__\0"
	"gpio_to_desc\0"
	"gpiod_get_raw_value\0"
	"gpiod_set_raw_value\0"
	"gpio_request\0"
	"gpiod_direction_input\0"
	"gpiod_direction_output_raw\0"
	"gpiod_to_irq\0"
	"request_threaded_irq\0"
	"free_irq\0"
	"gpio_free\0"
	"_printk\0"
	"module_layout\0"
;

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "47DC420CD2320AED9F103F1");
