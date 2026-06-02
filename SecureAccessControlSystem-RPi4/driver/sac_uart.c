// SPDX-License-Identifier: GPL-2.0
/*
 * sac_uart.c — PL011 UART0 on BCM2711 via ioremap (GPIO14 TX / GPIO15 RX).
 *
 * Exposes misc device /dev/uart for audit export (write-only path used by app).
 * Linked into secure_access_core.ko; registered from sac_probe().
 *
 * IMPORTANT: Do not use the kernel ttyAMA0 driver on the same UART at the same time.
 * Disable serial console / serial-getty and use only this node for audit TX, or
 * unload/disable the platform PL011 tty before insmod.
 *
 * Default baud: 9600 (fixed; match minicom).
 */

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

int sac_uart_register(void);
void sac_uart_unregister(void);

/* BCM2711 (Pi 4) physical addresses */
#define GPIO_BASE	0xFE200000
#define UART0_BASE	0xFE201000
#define UART0_SIZE	0x1000
#define GPIO_SIZE	0x1000

#define UART_DR		0x00
#define UART_FR		0x18
#define UART_IBRD	0x24
#define UART_FBRD	0x28
#define UART_LCRH	0x2C
#define UART_CR		0x30

#define UART_FR_TXFF	BIT(5)
#define UART_FR_RXFE	BIT(4)

#define SAC_UART_WRITE_MAX (512 * 1024)
#define UART_CLK_HZ	48000000U

static void __iomem *uart_base;
static void __iomem *gpio_base;
static DEFINE_MUTEX(uart_lock);
static bool uart_hw_up;

static const unsigned int uart_baud = 9600;

static void uart_mmio_send_char(char c)
{
	if (!uart_base)
		return;

	while (readl(uart_base + UART_FR) & UART_FR_TXFF)
		cpu_relax();

	writel((unsigned int)(unsigned char)c, uart_base + UART_DR);
}

static void uart_mmio_send_buf(const char *buf, size_t len)
{
	size_t i;

	for (i = 0; i < len; i++)
		uart_mmio_send_char(buf[i]);
}

/* GPIO14/15 = ALT0 (function 4) for UART0 TX/RX */
static int uart_gpio_mux(void)
{
	u32 val;

	if (!gpio_base)
		return -ENODEV;

	val = readl(gpio_base + 0x04);
	val &= ~((7U << 12) | (7U << 15));
	val |= (4U << 12) | (4U << 15);
	writel(val, gpio_base + 0x04);
	return 0;
}

static void uart_compute_baud(unsigned int baud, unsigned int *ibrd, unsigned int *fbrd)
{
	unsigned int div;
	unsigned int remainder;

	if (!baud)
		baud = 9600;

	div = UART_CLK_HZ / (16U * baud);
	remainder = UART_CLK_HZ % (16U * baud);
	*ibrd = div;
	*fbrd = (remainder * 64U + (16U * baud) / 2U) / (16U * baud);
}

static int uart_hw_init(void)
{
	unsigned int ibrd, fbrd;

	if (!uart_base || !gpio_base)
		return -ENODEV;

	if (uart_gpio_mux() < 0)
		return -EIO;

	uart_compute_baud(uart_baud, &ibrd, &fbrd);

	/* Disable UART while configuring */
	writel(0, uart_base + UART_CR);

	writel(ibrd, uart_base + UART_IBRD);
	writel(fbrd, uart_base + UART_FBRD);
	/* 8N1, FIFO enabled */
	writel((3U << 5) | (1U << 4), uart_base + UART_LCRH);
	/* Enable UART, TX + RX */
	writel((1U << 0) | (1U << 8) | (1U << 9), uart_base + UART_CR);

	uart_hw_up = true;
	return 0;
}

static void uart_hw_shutdown(void)
{
	if (uart_base)
		writel(0, uart_base + UART_CR);
	uart_hw_up = false;
}

static int uart_map_mmio(void)
{
	if (uart_base && gpio_base)
		return 0;

	gpio_base = ioremap(GPIO_BASE, GPIO_SIZE);
	if (!gpio_base)
		return -ENOMEM;

	uart_base = ioremap(UART0_BASE, UART0_SIZE);
	if (!uart_base) {
		iounmap(gpio_base);
		gpio_base = NULL;
		return -ENOMEM;
	}

	return 0;
}

static void uart_unmap_mmio(void)
{
	uart_hw_shutdown();
	if (uart_base) {
		iounmap(uart_base);
		uart_base = NULL;
	}
	if (gpio_base) {
		iounmap(gpio_base);
		gpio_base = NULL;
	}
}

static ssize_t sac_uart_write(struct file *file, const char __user *buf,
			      size_t len, loff_t *ppos)
{
	void *kbuf;
	ssize_t ret;

	(void)file;
	(void)ppos;

	if (!len)
		return 0;
	if (len > SAC_UART_WRITE_MAX)
		return -EINVAL;
	if (!uart_hw_up)
		return -ENODEV;

	kbuf = kvmalloc(len, GFP_KERNEL);
	if (!kbuf)
		return -ENOMEM;
	if (copy_from_user(kbuf, buf, len)) {
		kvfree(kbuf);
		return -EFAULT;
	}

	mutex_lock(&uart_lock);
	uart_mmio_send_buf(kbuf, len);
	ret = (ssize_t)len;
	mutex_unlock(&uart_lock);

	kvfree(kbuf);
	return ret;
}

static const struct file_operations sac_uart_fops = {
	.owner = THIS_MODULE,
	.write = sac_uart_write,
};

static struct miscdevice sac_uart_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "uart",
	.fops = &sac_uart_fops,
};

int sac_uart_register(void)
{
	int ret;

	if (uart_hw_up)
		return 0;

	ret = uart_map_mmio();
	if (ret)
		return ret;

	ret = uart_hw_init();
	if (ret) {
		uart_unmap_mmio();
		return ret;
	}

	ret = misc_register(&sac_uart_misc);
	if (ret) {
		uart_unmap_mmio();
		return ret;
	}

	pr_info("sac_uart: /dev/uart ready @ %u baud (PL011 MMIO)\n", uart_baud);
	return 0;
}

void sac_uart_unregister(void)
{
	misc_deregister(&sac_uart_misc);
	mutex_lock(&uart_lock);
	uart_unmap_mmio();
	mutex_unlock(&uart_lock);
}
