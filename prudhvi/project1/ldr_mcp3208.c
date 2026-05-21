#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include<linux/delay.h>
#include<linux/timer.h>
#include<linux/jiffies.h>
#include<linux/workqueue.h>
#include<linux/gpio.h>
#include<linux/err.h>
/* ---------- Module Info ---------- */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
MODULE_DESCRIPTION("MCP3208 LDR SPI Driver");
#define GPIO_LED (17 + 512)
#define uart_base_phys 0XFE201000
#define uart_size 1000

#define UART_DR 0X00
#define UART_FR 0X18
#define UART_IBRD 0x24
#define UART_FBRD 0x28
#define UART_LCRH 0x2C
#define UART_CR  0x30

#define TXFF 1<<5
#define RXFE 1<<4

#define UARTEN 1<<0
#define TXE 1<<8
#define RXE 1<<9

/* ---------- Device Name ---------- */
#define DEVICE_NAME "ldr_adc"
#define TIMER_INTERVAL_MS 1000 //1 SEC

static struct spi_device *ldr_spi;
static struct timer_list my_timer;
static void __iomem *uart_base;
static struct work_struct my_work;

/*------------Uart init------------*/
static void uart_init(void)
{
	// Disable UART
	writel(0x0, uart_base + UART_CR);

	// Baud rate: 9600 (assuming 48MHz clock)
	writel(312, uart_base + UART_IBRD);
	writel(32,  uart_base + UART_FBRD);

	// 8-bit, no parity, 1 stop bit
	writel((3 << 5), uart_base + UART_LCRH);

	// Enable UART, TX, RX
	writel(UARTEN | TXE | RXE, uart_base + UART_CR);

	pr_info("UART initialized\n");
}
static void led_config(void)
{
// Request the GPIO
	if (!gpio_is_valid(GPIO_LED)) {
		pr_err("LED: Invalid GPIO %d\n", GPIO_LED);
    }

    if (gpio_request(GPIO_LED, DEVICE_NAME)) {
        pr_err("LED: Failed to request GPIO %d\n", GPIO_LED);
    }

    gpio_direction_output(GPIO_LED, 0);  // Set as output, initial LOW

}
// ---------------- SEND CHAR ----------------
static void uart_send_char(char c)
{
	// Wait until TX FIFO not full
	while (readl(uart_base + UART_FR) & TXFF);

	writel(c, uart_base + UART_DR);
}
// ---------------- SEND STRING ----------------
static void uart_send_string(const char *str)
{
	while (*str) {
		uart_send_char(*str++);
	}
}


/* ---------- SPI Read MCP3208 ---------- */
static int mcp3208_read_channel(u8 channel)
{
	u8 tx[3];
	u8 rx[3];

	struct spi_transfer t = {
		.tx_buf = tx,
		.rx_buf = rx,
		.len = 3,
	};

	struct spi_message m;

	/* MCP3208 command format */
	tx[0] = 0x06 | ((channel & 0x04) >> 2);
	tx[1] = (channel & 0x03) << 6;
	tx[2] = 0x00;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);

	if (spi_sync(ldr_spi, &m) < 0) {
		pr_err("SPI transfer failed\n");
		return -EIO;
	}

	return ((rx[1] & 0x0F) << 8) | rx[2];
}

char buffer[32];
int flag;
static void my_work_handler(struct work_struct *work)

{
	int adc_ldr;
	int adc_temp;
	int ret;


	adc_ldr = mcp3208_read_channel(0);
	if (adc_ldr < 0) return;
	adc_temp = mcp3208_read_channel(1);
	if (adc_temp < 0) return;
	pr_info("LDR ADC Value = %d   temparature ADC value %d\n", adc_ldr,adc_temp);
	if((adc_ldr<=100)&&!flag)
	{
		gpio_set_value(GPIO_LED,1);
		pr_info("LED:ON\n");
		flag=1;
	}
	else if((adc_ldr>100)&&(adc_ldr<4096)&&flag)
	{
		gpio_set_value(GPIO_LED,0);
		pr_info("LED:OFF\n");
		flag=0;
	}


	ret = snprintf(buffer, sizeof(buffer), "%d %d", adc_ldr,adc_temp);

	uart_send_string(buffer);


}
static void my_timer_callback(struct timer_list *t)
{
	schedule_work(&my_work);
	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));
}

/* ---------- Device Tree Match ---------- */
static const struct of_device_id ldr_dt_ids[] = {
	{ .compatible = "custom,ldr-mcp3208" },
	{ }
};

MODULE_DEVICE_TABLE(of, ldr_dt_ids);

/* ---------- SPI ID Table (fix warning) ---------- */
static const struct spi_device_id ldr_spi_id[] = {
	{ "ldr-mcp3208", 0 },
	{ }
};

MODULE_DEVICE_TABLE(spi, ldr_spi_id);

/* ---------- Probe Function ---------- */
static int ldr_probe(struct spi_device *spi)
{
	int ret;

	pr_info("LDR SPI Driver Probe\n");

	ldr_spi = spi;

	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;
	spi->max_speed_hz = 1000000;

	ret = spi_setup(spi);
	if (ret) {
		pr_err("SPI setup failed\n");
		return ret;
	}
	uart_base=ioremap(uart_base_phys,uart_size);
	if(!uart_base)
	{
		pr_err("Failed to map UART\n");
		return -ENOMEM;
	}
	uart_init();
	// Send test message
	uart_send_string("LDR and Tempareture data\n\r");
led_config();
	INIT_WORK(&my_work,my_work_handler);
	timer_setup(&my_timer,my_timer_callback,0);

	mod_timer(&my_timer,jiffies +msecs_to_jiffies(TIMER_INTERVAL_MS));

	pr_info("SPI:STARTED FEATCHING DATA\n");

	return 0;
}

/* ---------- Remove Function ---------- */
static void ldr_remove(struct spi_device *spi)
{
	del_timer_sync(&my_timer);
	flush_work(&my_work);
	// Disable UART
	writel(0x0, uart_base + UART_CR);

	iounmap(uart_base);
gpio_set_value(GPIO_LED, 0);  // Turn off LED
    gpio_free(GPIO_LED);

	pr_info("LDR Driver Removed\n");
}

/* ---------- SPI Driver ---------- */
static struct spi_driver ldr_driver = {
	.driver = {
		.name = "ldr_mcp3208",
		.of_match_table = ldr_dt_ids,
	},
	.id_table = ldr_spi_id,
	.probe = ldr_probe,
	.remove = ldr_remove,
};

module_spi_driver(ldr_driver);

