#include <linux/module.h>
#include <linux/spi/spi.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

/* ---------- Device Name ---------- */
#define DEVICE_NAME "ldr_adc"

static struct spi_device *ldr_spi;
static int major;
static struct class *ldr_class;

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

/* ---------- File Read ---------- */
static ssize_t ldr_read(struct file *file,
                        char __user *buf,
                        size_t len,
                        loff_t *offset)
{
    char buffer[32];
    int adc_ldr;
    int adc_temp;
    int ret;

    //    if (*offset > 0)
    //      return 0;

	    adc_ldr = mcp3208_read_channel(0);
	    adc_temp = mcp3208_read_channel(1);

	    pr_info("LDR ADC Value = %d   temparature ADC value %d\n", adc_ldr,adc_temp);

	    ret = snprintf(buffer, sizeof(buffer), "ldr %d  temparature=%d\n", adc_ldr,adc_temp);

	    if (copy_to_user(buf, buffer, ret))
		    return -EFAULT;
    
   // *offset += ret;

    return ret;
}

/* ---------- File Operations ---------- */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = ldr_read,
};

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

    /* register char device */
    major = register_chrdev(0, DEVICE_NAME, &fops);
    if (major < 0) {
        pr_err("Failed to register char device\n");
        return major;
    }

    ldr_class = class_create(DEVICE_NAME);
    if (IS_ERR(ldr_class)) {
        unregister_chrdev(major, DEVICE_NAME);
        return PTR_ERR(ldr_class);
    }

    device_create(ldr_class, NULL, MKDEV(major, 0), NULL, DEVICE_NAME);

    pr_info("/dev/ldr_adc created\n");

    return 0;
}

/* ---------- Remove Function ---------- */
static void ldr_remove(struct spi_device *spi)
{
    device_destroy(ldr_class, MKDEV(major, 0));
    class_destroy(ldr_class);
    unregister_chrdev(major, DEVICE_NAME);

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

/* ---------- Module Info ---------- */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prudhvi");
MODULE_DESCRIPTION("MCP3208 LDR SPI Driver");
