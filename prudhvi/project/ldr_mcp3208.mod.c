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
	{ 0x24b77ab9, "__spi_register_driver" },
	{ 0x92893115, "driver_unregister" },
	{ 0xdcb764ad, "memset" },
	{ 0x9e533066, "spi_sync" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x92997ed8, "_printk" },
	{ 0x2d3385d3, "system_wq" },
	{ 0xc5b6f236, "queue_work_on" },
	{ 0x15ba50a6, "jiffies" },
	{ 0xc38c83b8, "mod_timer" },
	{ 0xeae3dfd6, "__const_udelay" },
	{ 0x82ee90dc, "timer_delete_sync" },
	{ 0x2f2c95c4, "flush_work" },
	{ 0xedc03953, "iounmap" },
	{ 0xf9a482f9, "msleep" },
	{ 0x138cd40d, "gpio_to_desc" },
	{ 0xacc895c0, "gpiod_set_raw_value" },
	{ 0xfe990052, "gpio_free" },
	{ 0x656e4a6e, "snprintf" },
	{ 0xd293d82e, "gpiod_get_raw_value" },
	{ 0xf2b3fa4, "spi_setup" },
	{ 0xaf56600a, "arm64_use_ng_mappings" },
	{ 0x40863ba1, "ioremap_prot" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x634f3ea8, "gpiod_direction_output_raw" },
	{ 0xc6f46339, "init_timer_key" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:ldr-mcp3208");
MODULE_ALIAS("of:N*T*Ccustom,ldr-mcp3208");
MODULE_ALIAS("of:N*T*Ccustom,ldr-mcp3208C*");

MODULE_INFO(srcversion, "21952CDECC2BC7998352252");
