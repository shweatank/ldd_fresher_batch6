//bcm2711_gpio_uart.c
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/fs.h>

#define DRIVER_NAME "bcm2711_uart"
#define DEVICE_NAME "uart"
#define CLASS_NAME "uart"

// BCM2711 GPIO base
#define GPIO_BASE        0xFE200000
#define UART0_BASE       0xFE201000
#define UART0_SIZE       0x1000

// Register offsets
#define UART_DR      0x00
#define UART_FR      0x18
#define UART_IBRD    0x24
#define UART_FBRD    0x28
#define UART_LCRH    0x2C
#define UART_CR      0x30
#define UART_IMSC    0x38

//Flag bits
#define TXFF (1<<5) //Transmit FIFO Full
#define RXFE (1<<4) //Receive FIFO Empty
static void __iomem *uart_base;
static void __iomem *gpio_base;
static int major;
static struct class* uart_class = NULL;
static struct device* uart_device = NULL;

static int uart_open(struct inode *inodep, struct file *filep)
{
        pr_info("UART: Device opened\n");
        return 0;
}

static int uart_release(struct inode *inodep, struct file *filep)
{
        pr_info("UART: Device closed\n");
        return 0;
}

//Configure UART Initialization
static void  uart_init(void)
{
      uart_base = ioremap(UART0_BASE, UART0_SIZE);
      gpio_base = ioremap(GPIO_BASE,0x1000);

      if(!uart_base)
      {
	      pr_info("UART ioremap failed\n");
	      return ;
      }
      if(!gpio_base)
      {
              pr_info("GPIO ioremap failed\n");
              return ;
      }

      unsigned int val;

      val= ioread32(gpio_base + 0x04);

      val&= ~((7 <<12) | (7 <<15));

      val |= (4<<12) | (4 <<15);

      iowrite32(val,gpio_base + 0x04);

      //Disable UART
      iowrite32(0x0, uart_base + UART_CR);

      //Baud rate (115200 for 48MHz)
      iowrite32(26, uart_base + UART_IBRD);
      iowrite32(3, uart_base + UART_FBRD);

    // 8-bit, FIFO enabled
    iowrite32((3 << 5) | (1 << 4), uart_base + UART_LCRH);

    // Enable UART, TX, RX
    iowrite32((1 << 0) | (1 << 8) | (1 << 9), uart_base + UART_CR);

    pr_info("UART: initialized\n");
}


// Send one character
static void uart_send_char(char c)
{
    // Wait until TX FIFO not full
    while (ioread32(uart_base + UART_FR) & TXFF);

    iowrite32(c, uart_base + UART_DR);
}


// Send string
static void uart_send_string(const char *str)
{
    while (*str) {
        uart_send_char(*str++);
    }
}


//Receive data from UART
static ssize_t uart_read(struct file *filep, char __user *buffer, size_t len, loff_t *offset)
{
    pr_info("UART:Reading from UART");
    char kbuf[64]; // Temporary kernel buffer
    size_t bytes_to_read;
    int count = 0;

    // 1. Determine how many bytes we can safely read into our temp buffer
    bytes_to_read = (len < sizeof(kbuf)) ? len : sizeof(kbuf);

    // 2. Loop until the hardware FIFO is empty OR we fill our temp buffer
    // RXFE (Receive FIFO Empty) is 1 when there is NO data
    while (!(ioread32(uart_base + UART_FR) & RXFE)) {
        kbuf[count++] = (char)(ioread32(uart_base + UART_DR) & 0xFF);
    }

    // 3. If no data was available at all, return 0 (non-blocking)
    if (count == 0) {
        return -EAGAIN;
    }

    // 4. Copy the entire string of characters to the user at once
    if (copy_to_user(buffer, kbuf, count)) {
        return -EFAULT;
    }

    // 5. Return the total number of characters read
    return count;
    pr_info("UART:Read Completed");
}



static ssize_t uart_write(struct file *filep, const char *buffer, size_t len, loff_t *offset)
{
	pr_info("UART:Writing into UART");
        char kbuf[32];

        size_t max_len = (len < sizeof(kbuf) - 1) ? len : sizeof(kbuf) - 1;

        if (copy_from_user(kbuf, buffer, max_len))
                return -EFAULT;

	kbuf[max_len] = '\0';

        uart_send_string(kbuf);
        return len;
	pr_info("UART:Writing Completed");
}

static struct file_operations fops = {
        .owner = THIS_MODULE,
        .open = uart_open,
        .write = uart_write,
	.read = uart_read,
        .release = uart_release,
};


// Module Init
static int __init uart_module_init(void)
{
     uart_init();

    //uart_send_string("UART Ready\r\n");
    //uart_send_string("Type characters (press 'q' to exit):\r\n");

    major = register_chrdev(0, DEVICE_NAME, &fops);
        if(major < 0)
        {
                pr_err("UART: Failed to register major number\n");
                return major;
        }

        uart_class= class_create(CLASS_NAME);
        if(IS_ERR(uart_class))
        {
                unregister_chrdev(major, DEVICE_NAME);
                pr_err("UART: Failed to create class\n");
                return PTR_ERR(uart_class);
        }

        uart_device = device_create(uart_class, NULL, MKDEV(major, 0 ), NULL, DEVICE_NAME);
        if(IS_ERR(uart_device))
        {
                class_destroy(uart_class);
                unregister_chrdev(major, DEVICE_NAME);
                pr_err("UART: Failed to create device\n");
                return PTR_ERR(uart_device);
        }

        pr_info("UART: Driver loaded. Use /dev/%s\n",DEVICE_NAME);
	uart_send_string("UART Ready \n");

    return 0;
}


//  Module Exit
static void __exit uart_module_exit(void)
{
    if (uart_base) {
        iounmap(uart_base);
        iounmap(gpio_base);
	device_destroy(uart_class, MKDEV(major, 0));
        class_unregister(uart_class);
        class_destroy(uart_class);
        unregister_chrdev(major, DEVICE_NAME);
        printk("UART unmapped\n");
    }

}

module_init(uart_module_init);
module_exit(uart_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("UART TX/RX Driver using ioremap (Polling)");
