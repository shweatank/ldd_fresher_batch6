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
	{ 0x607a5c68, "device_destroy" },
	{ 0x6775d5d3, "class_destroy" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0x92997ed8, "_printk" },
	{ 0xdcb764ad, "memset" },
	{ 0x9e533066, "spi_sync" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0xf2b3fa4, "spi_setup" },
	{ 0x418c10ec, "__register_chrdev" },
	{ 0x59c02473, "class_create" },
	{ 0x6e26cac4, "device_create" },
	{ 0x92893115, "driver_unregister" },
	{ 0x656e4a6e, "snprintf" },
	{ 0x6cbbfc54, "__arch_copy_to_user" },
	{ 0x474e54d2, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("spi:ldr-mcp3208");
MODULE_ALIAS("of:N*T*Ccustom,ldr-mcp3208");
MODULE_ALIAS("of:N*T*Ccustom,ldr-mcp3208C*");

MODULE_INFO(srcversion, "4789525418668D02DC47BD8");
