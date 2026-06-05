#include"i2c_header.h"
// SPDX-License-Identifier: GPL-2.0
/*
 * A simple Linux kernel module to demonstrate GPIO interrupts and UART communication.
 * This module listens for interrupts from a GPIO pin (connected to an IR sensor) and
 * sends data over UART when the interrupt occurs. It also provides a character device
 * interface for user-space interaction.
 */

/* This pointer stores the I2C device information for OLED */
static struct i2c_client *oled_client;

/* This stores the major number given by kernel for character device */
static int major;

/* This class is used to create /dev/oled device file */
static struct class *oled_class;

/* workqueue for deferred OLED updates */
static unsigned int clear_delay_ms = 5000;   /* 5s */

/* This delayed work is used to clear OLED after some delay */
static struct delayed_work oled_clear_work;


/*
 * This table contains pixel pattern for ASCII characters.
 * Each character is stored as 5 bytes.
 * OLED uses this data to display letters, numbers and symbols.
 */
static const unsigned char font5x7[95][5] = {
{0x00,0x00,0x00,0x00,0x00}, // 32  Space
{0x00,0x00,0x5F,0x00,0x00}, // 33 !
{0x00,0x07,0x00,0x07,0x00}, // 34 "
{0x14,0x7F,0x14,0x7F,0x14}, // 35 #
{0x24,0x2A,0x7F,0x2A,0x12}, // 36 $
{0x23,0x13,0x08,0x64,0x62}, // 37 %
{0x36,0x49,0x55,0x22,0x50}, // 38 &
{0x00,0x05,0x03,0x00,0x00}, // 39 '
{0x00,0x1C,0x22,0x41,0x00}, // 40 (
{0x00,0x41,0x22,0x1C,0x00}, // 41 )
{0x14,0x08,0x3E,0x08,0x14}, // 42 *
{0x08,0x08,0x3E,0x08,0x08}, // 43 +
{0x00,0x50,0x30,0x00,0x00}, // 44 ,
{0x08,0x08,0x08,0x08,0x08}, // 45 -
{0x00,0x60,0x60,0x00,0x00}, // 46 .
{0x20,0x10,0x08,0x04,0x02}, // 47 /
{0x3E,0x51,0x49,0x45,0x3E}, // 48 0
{0x00,0x42,0x7F,0x40,0x00}, // 49 1
{0x42,0x61,0x51,0x49,0x46}, // 50 2
{0x21,0x41,0x45,0x4B,0x31}, // 51 3
{0x18,0x14,0x12,0x7F,0x10}, // 52 4
{0x27,0x45,0x45,0x45,0x39}, // 53 5
{0x3C,0x4A,0x49,0x49,0x30}, // 54 6
{0x01,0x71,0x09,0x05,0x03}, // 55 7
{0x36,0x49,0x49,0x49,0x36}, // 56 8
{0x06,0x49,0x49,0x29,0x1E}, // 57 9
{0x00,0x36,0x36,0x00,0x00}, // 58 :
{0x00,0x56,0x36,0x00,0x00}, // 59 ;
{0x08,0x14,0x22,0x41,0x00}, // 60 <
{0x14,0x14,0x14,0x14,0x14}, // 61 =
{0x00,0x41,0x22,0x14,0x08}, // 62 >
{0x02,0x01,0x51,0x09,0x06}, // 63 ?
{0x32,0x49,0x79,0x41,0x3E}, // 64 @
{0x7E,0x11,0x11,0x11,0x7E}, // 65 A
{0x7F,0x49,0x49,0x49,0x36}, // 66 B
{0x3E,0x41,0x41,0x41,0x22}, // 67 C
{0x7F,0x41,0x41,0x22,0x1C}, // 68 D
{0x7F,0x49,0x49,0x49,0x41}, // 69 E
{0x7F,0x09,0x09,0x09,0x01}, // 70 F
{0x3E,0x41,0x49,0x49,0x7A}, // 71 G
{0x7F,0x08,0x08,0x08,0x7F}, // 72 H
{0x00,0x41,0x7F,0x41,0x00}, // 73 I
{0x20,0x40,0x41,0x3F,0x01}, // 74 J
{0x7F,0x08,0x14,0x22,0x41}, // 75 K
{0x7F,0x40,0x40,0x40,0x40}, // 76 L
{0x7F,0x02,0x0C,0x02,0x7F}, // 77 M
{0x7F,0x04,0x08,0x10,0x7F}, // 78 N
{0x3E,0x41,0x41,0x41,0x3E}, // 79 O
{0x7F,0x09,0x09,0x09,0x06}, // 80 P
{0x3E,0x41,0x51,0x21,0x5E}, // 81 Q
{0x7F,0x09,0x19,0x29,0x46}, // 82 R
{0x46,0x49,0x49,0x49,0x31}, // 83 S
{0x01,0x01,0x7F,0x01,0x01}, // 84 T
{0x3F,0x40,0x40,0x40,0x3F}, // 85 U
{0x1F,0x20,0x40,0x20,0x1F}, // 86 V
{0x3F,0x40,0x38,0x40,0x3F}, // 87 W
{0x63,0x14,0x08,0x14,0x63}, // 88 X
{0x07,0x08,0x70,0x08,0x07}, // 89 Y
{0x61,0x51,0x49,0x45,0x43}, // 90 Z
{0x00,0x7F,0x41,0x41,0x00}, // 91 [
{0x02,0x04,0x08,0x10,0x20}, // 92 
{0x00,0x41,0x41,0x7F,0x00}, // 93 ]
{0x04,0x02,0x01,0x02,0x04}, // 94 ^
{0x40,0x40,0x40,0x40,0x40}, // 95 _
{0x00,0x01,0x02,0x04,0x00}, // 96 `
{0x20,0x54,0x54,0x54,0x78}, // 97 a
{0x7F,0x48,0x44,0x44,0x38}, // 98 b
{0x38,0x44,0x44,0x44,0x20}, // 99 c
{0x38,0x44,0x44,0x48,0x7F}, //100 d
{0x38,0x54,0x54,0x54,0x18}, //101 e
{0x08,0x7E,0x09,0x01,0x02}, //102 f
{0x0C,0x52,0x52,0x52,0x3E}, //103 g
{0x7F,0x08,0x04,0x04,0x78}, //104 h
{0x00,0x44,0x7D,0x40,0x00}, //105 i
{0x20,0x40,0x44,0x3D,0x00}, //106 j
{0x7F,0x10,0x28,0x44,0x00}, //107 k
{0x00,0x41,0x7F,0x40,0x00}, //108 l
{0x7C,0x04,0x18,0x04,0x78}, //109 m
{0x7C,0x08,0x04,0x04,0x78}, //110 n
{0x38,0x44,0x44,0x44,0x38}, //111 o
{0x7C,0x14,0x14,0x14,0x08}, //112 p
{0x08,0x14,0x14,0x18,0x7C}, //113 q
{0x7C,0x08,0x04,0x04,0x08}, //114 r
{0x48,0x54,0x54,0x54,0x20}, //115 s
{0x04,0x3F,0x44,0x40,0x20}, //116 t
{0x3C,0x40,0x40,0x20,0x7C}, //117 u
{0x1C,0x20,0x40,0x20,0x1C}, //118 v
{0x3C,0x40,0x30,0x40,0x3C}, //119 w
{0x44,0x28,0x10,0x28,0x44}, //120 x
{0x0C,0x50,0x50,0x50,0x3C}, //121 y
{0x44,0x64,0x54,0x4C,0x44}  //122 z
};


/*
 * This function sends one command byte to OLED.
 * 0x00 means the next byte is command.
 */
static void oled_cmd(u8 cmd)
{
	u8 buf[2] = {0x00, cmd};

	i2c_master_send(oled_client, buf, 2);
}

/*
 * This function sends one data byte to OLED.
 * 0x40 means the next byte is display data.
 */
static void oled_data(u8 data)
{
	u8 buf[2] = {0x40, data};

	i2c_master_send(oled_client, buf, 2);
}

/*
 * This function initializes SSD1306 OLED display.
 * These commands configure display size, addressing mode,
 * contrast, charge pump and turn ON the display.
 */
static void oled_init_display(void)
{
	oled_cmd(0xAE);                 /* display OFF */
	oled_cmd(0xD5); oled_cmd(0x80); /* clock setting */
	oled_cmd(0xA8); oled_cmd(0x1F); /* 128x32 display */
	oled_cmd(0xD3); oled_cmd(0x00); /* display offset */
	oled_cmd(0x40);                 /* start line */
	oled_cmd(0x8D); oled_cmd(0x14); /* charge pump enable */
	oled_cmd(0x20); oled_cmd(0x00); /* horizontal addressing mode */
	oled_cmd(0xA1);                 /* segment remap */
	oled_cmd(0xC8);                 /* COM scan direction */
	oled_cmd(0xDA); oled_cmd(0x02); /* COM pins setting */
	oled_cmd(0x81); oled_cmd(0x8F); /* contrast */
	oled_cmd(0xD9); oled_cmd(0xF1); /* pre-charge period */
	oled_cmd(0xDB); oled_cmd(0x40); /* VCOMH level */
	oled_cmd(0xA4);                 /* display from RAM */
	oled_cmd(0xA6);                 /* normal display */
	oled_cmd(0xAF);                 /* display ON */
}

/*
 * This function clears full OLED screen.
 * 128x32 OLED has 512 bytes display RAM.
 */
static void oled_clear(void)
{
	int i;

	/* Set column address from 0 to 127 */
	oled_cmd(0x21); oled_cmd(0x00); oled_cmd(0x7F);

	/* Set page address from 0 to 3 */
	oled_cmd(0x22); oled_cmd(0x00); oled_cmd(0x03);

	/* Send zero to all OLED display memory */
	for (i = 0; i < 512; i++)
		oled_data(0x00);
}


//clear function to be scheduled in workqueue context(process context) to safely perform I2C operations
static void oled_clear_work_fn(struct work_struct *work)
{
	/* Runs in workqueue context (can sleep), safe for I2C */

	/* Clear OLED after delay time is completed */
	oled_clear();
}

/*
 * This function prints one character on OLED.
 * It takes character pattern from font5x7 table.
 */
static void oled_print_char(char c)
{
	int i;

	/* Ignore unsupported characters */
	if (c < 32 || c > 126)
		return;

	/* Send 5 columns of character font */
	for (i = 0; i < 5; i++)
		oled_data(font5x7[c - 32][i]);

	/* Add one empty column as space between characters */
	oled_data(0x00);
}

/*
 * This function is called when user writes data to /dev/oled.
 * Example: echo HELLO > /dev/oled
 */
static ssize_t oled_write(struct file *file,
			  const char __user *buf,
			  size_t len,
			  loff_t *offset)
{
	char kbuf[32];
	int i;

	/* Limit input size to buffer size */
	if (len > sizeof(kbuf) - 1)
		len = sizeof(kbuf) - 1;

	/* Copy data from user space to kernel space */
	if (copy_from_user(kbuf, buf, len))
		return -EFAULT;

	/* Add string ending */
	kbuf[len] = '\0';

	/* Clear old text before printing new text */
	oled_clear();

	/* Print each character one by one */
	for (i = 0; i < len; i++)
		oled_print_char(kbuf[i]);

	/* Reset the clear timer on every new write */
	cancel_delayed_work_sync(&oled_clear_work);
	schedule_delayed_work(&oled_clear_work, msecs_to_jiffies(clear_delay_ms));

	/* Return number of bytes written */
	return len;
}

/*
 * File operations for character device.
 * Here only write operation is used.
 */
static const struct file_operations oled_fops = {
	.owner = THIS_MODULE,
	.write = oled_write,
};

/*
 * Probe function is called when device tree compatible string matches.
 * This function initializes OLED and creates /dev/oled.
 */
static int oled_probe(struct i2c_client *client)
{
	int ret;

	/* Save I2C client pointer globally */
	oled_client = client;

	/* Initialize OLED display */
	oled_init_display();
	msleep(100);
	oled_clear();

	//INITIALIZE WORKQUEUE (delayed workqueue for deferred clear which has kernel timers inside)
	INIT_DELAYED_WORK(&oled_clear_work, oled_clear_work_fn);

	/* Register character device and get major number */
	major = register_chrdev(0, DEVICE_NAME, &oled_fops);
	pr_info("the major number:%d\n", major);

	if (major < 0)
		return major;

	/* Create device class */
	oled_class = class_create(DEVICE_NAME);
	if (IS_ERR(oled_class)) {
		ret = PTR_ERR(oled_class);
		unregister_chrdev(major, DEVICE_NAME);
		return ret;
	}

	/* Create /dev/oled device file */
	device_create(oled_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

	pr_info("OLED DTS I2C driver loaded at addr 0x%02x\n", client->addr);
	return 0;
}

/*
 * Remove function is called when driver is removed.
 * It deletes device file and unregisters character device.
 */
static void oled_remove(struct i2c_client *client)
{
	//cancel the deelayed workqueue
	cancel_delayed_work_sync(&oled_clear_work);

	/* Remove /dev/oled */
	device_destroy(oled_class, MKDEV(major, 0));

	/* Destroy class */
	class_destroy(oled_class);

	/* Unregister character device */
	unregister_chrdev(major, DEVICE_NAME);

	pr_info("OLED DTS I2C driver removed\n");
}

/*
 * Device tree matching table.
 * Compatible string in DTS must be same as this.
 */
static const struct of_device_id oled_of_match[] = {
	{ .compatible = "siddarth,oled-ssd1306" },
	{ }
};

MODULE_DEVICE_TABLE(of, oled_of_match);

/*
 * I2C device ID table.
 * This is used for normal I2C driver matching.
 */
static const struct i2c_device_id oled_id[] = {
	{ "oled-ssd1306", 0 },
	{ }
};

MODULE_DEVICE_TABLE(i2c, oled_id);

/*
 * I2C driver structure.
 * Kernel uses this structure to call probe and remove functions.
 */
static struct i2c_driver oled_driver = {
	.driver = {
		.name = "oled-ssd1306",
		.of_match_table = oled_of_match,
	},
	.probe = oled_probe,
	.remove = oled_remove,
	.id_table = oled_id,
};

/*
 * This macro creates module init and exit functions automatically
 * for this I2C driver.
 */
module_i2c_driver(oled_driver);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("siddarth");
MODULE_DESCRIPTION("DTS based I2C OLED SSD1306 character driver");
