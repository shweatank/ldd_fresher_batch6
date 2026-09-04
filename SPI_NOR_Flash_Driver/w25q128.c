#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>
/* Driver name */
#define DRIVER_NAME "w25q128"

#define W25Q128_CMD_READ             0x03
#define W25Q128_CMD_JEDEC_ID         0x9F
#define W25Q128_CMD_WRITE_ENABLE     0x06
#define W25Q128_CMD_READ_STATUS      0x05
#define W25Q128_CMD_PAGE_PROGRAM     0x02
#define W25Q128_CMD_SECTOR_ERASE     0x20

#define W25Q128_FLASH_SIZE           (16 * 1024 * 1024)
#define W25Q128_SECTOR_SIZE          (4 * 1024)
#define W25Q128_PAGE_SIZE            256
#define W25Q128_STATUS_BUSY          0x01



struct w25q128 {
	struct spi_device *spi;
	struct mtd_info mtd;
	struct mutex lock;
};


//write enable means operation is takes place write/erase enabling the hardware WEL bit 0 -> 1
static int w25q128_write_enable(struct spi_device *spi)
{
    u8 cmd =W25Q128_CMD_WRITE_ENABLE ;

    return spi_write(spi, &cmd, 1);
}

//We need to know whether the flash is busy. 
static int w25q128_read_status(struct spi_device *spi, u8 *status)
{
    u8 cmd = W25Q128_CMD_READ_STATUS;
    int ret;

    ret = spi_write_then_read(spi,
                              &cmd,
                              1,
                              status,
                              1);

    return ret;
}

//if operation is happening at that time we will check the busy flag
static int w25q128_wait_ready(struct spi_device *spi)
{
    u8 status;
    int ret;

    do {
        ret = w25q128_read_status(spi, &status);

        if (ret)
            return ret;

        if (!(status & 0x01)) //first bit of status is 1 means operation is happening 
            break;

        usleep_range(1000, 2000);

    } while (1);

    return 0;
}

static int w25q128_page_program(struct w25q128 *dev,
                                loff_t addr,
                                const u8 *buf,
                                size_t len)
{
    u8 *tx;
    int ret;

    tx = kmalloc(4 + len, GFP_KERNEL);
    if (!tx)
        return -ENOMEM;

    tx[0] = W25Q128_CMD_PAGE_PROGRAM;
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;

    memcpy(&tx[4], buf, len);

    ret = spi_write(dev->spi, tx, 4 + len);

    kfree(tx);

    return ret;
}

static int w25q128_write(struct mtd_info *mtd,
                         loff_t to,
                         size_t len,
                         size_t *retlen,
                         const u_char *buf)
{
    struct w25q128 *dev;
    size_t written = 0;
    int ret = 0;

    dev = container_of(mtd, struct w25q128, mtd);

    *retlen = 0;

    if (to < 0 || to + len > mtd->size)
        return -EINVAL;

		/* Lock the flash */
    mutex_lock(&dev->lock);
    while (written < len) {

        loff_t addr = to + written;

        size_t page_off = addr % W25Q128_PAGE_SIZE;

        size_t chunk = W25Q128_PAGE_SIZE - page_off;

        if (chunk > len - written)
            chunk = len - written;

        /* Enable write */
        ret = w25q128_write_enable(dev->spi);
        if (ret < 0)
            break;

        /* Program page */
        ret = w25q128_page_program(dev,
                                   addr,
                                   buf + written,
                                   chunk);
        if (ret < 0)
            break;

        /* Wait until flash is ready */
        ret = w25q128_wait_ready(dev->spi);
        if (ret < 0)
            break;

        written += chunk;
    }

    /* Unlock the flash */
    mutex_unlock(&dev->lock);
    *retlen = written;

    if (!ret)
        {
			//printk(KERN_INFO "W25Q128 WRITE Done\n");
	dev_info(&dev->spi->dev,
         "W25Q128 WRITE Done\n");
		}

    return ret;
}

static int w25q128_sector_erase(struct w25q128 *dev, u32 addr)
{
    u8 cmd[4];
    int ret;

    cmd[0] = W25Q128_CMD_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    ret = w25q128_write_enable(dev->spi);
    if (ret < 0)
        return ret;

    ret = spi_write(dev->spi, cmd, sizeof(cmd));
    if (ret < 0)
        return ret;

    ret = w25q128_wait_ready(dev->spi);

    return ret;
}

static int w25q128_erase(struct mtd_info *mtd,
                         struct erase_info *instr)
{
    struct w25q128 *dev =
        container_of(mtd, struct w25q128, mtd);

    u32 addr = instr->addr;
    u32 len = instr->len;
    int ret;

    if (addr % mtd->erasesize ||
        len % mtd->erasesize)
        return -EINVAL;

    if ((u64)addr + len > mtd->size)
        return -EINVAL;

    mutex_lock(&dev->lock);

    while (len) {

        ret = w25q128_sector_erase(dev, addr);

        if (ret < 0) {
            instr->fail_addr = addr;
            mutex_unlock(&dev->lock);
            return ret;
        }

        addr += mtd->erasesize;
        len -= mtd->erasesize;
    }

    mutex_unlock(&dev->lock);

   dev_info(&dev->spi->dev,
         "SPI NOR ERASE Done\n");

    return 0;
}


/*
 * Detect W25Q128 using JEDEC ID command
 */
static int w25q128_detect(struct spi_device *spi)
{
	u8 cmd = W25Q128_CMD_JEDEC_ID;
	u8 id[3];
	int ret;

	ret = spi_write_then_read(spi, &cmd, 1, id, 3);

	if (ret) {
		dev_err(&spi->dev,
			"JEDEC ID read failed: %d\n", ret);
		return ret;
	}

	dev_info(&spi->dev,
		 "JEDEC ID: %02X %02X %02X\n",
		 id[0], id[1], id[2]);

	/*
	 * W25Q128 expected JEDEC ID:
	 *
	 * Manufacturer = EF
	 * Memory Type  = 40
	 * Capacity     = 18
	 */
	if (id[0] != 0xEF ||
	    id[1] != 0x40 ||
	    id[2] != 0x18) {

		dev_err(&spi->dev,
			"Unexpected JEDEC ID\n");

		return -ENODEV;
	}

	dev_info(&spi->dev,
		 "W25Q128 detected successfully\n");

	return 0;
}


/*
 * MTD READ operation
 *
 * This function is called when userspace/kernel
 * wants to read data from the SPI NOR flash.
 
 */
static int w25q128_read(struct mtd_info *mtd,
                        loff_t from,
                        size_t len,
                        size_t *retlen,
                        u_char *buf)
{
    struct w25q128 *flash;
    u8 cmd[4];
    int ret;

    flash = container_of(mtd, struct w25q128, mtd); 

    dev_info(&flash->spi->dev,
             "READ called: from=0x%llx len=%zu\n",
             from, len);

    if (from >= mtd->size)
        return -EINVAL;

    if (len > mtd->size - from)
        return -EINVAL;

    cmd[0] = W25Q128_CMD_READ;
    cmd[1] = (from >> 16) & 0xff;
    cmd[2] = (from >> 8) & 0xff;
    cmd[3] = from & 0xff;

   mutex_lock(&flash->lock);

ret = spi_write_then_read(flash->spi,
                          cmd,
                          4,
                          buf,
                          len);

mutex_unlock(&flash->lock);

    if (ret) {
        dev_err(&flash->spi->dev,
                "SPI read failed: %d\n", ret);
        return ret;
    }

    *retlen = len;

    dev_info(&flash->spi->dev,
             "SPI READ successful: %zu bytes\n",
             len);

    return 0;
}



 
static int w25q128_probe(struct spi_device *spi)
{
	struct w25q128 *flash;
	struct mtd_info *mtd;
	int ret;

	dev_info(&spi->dev, "W25Q128 probe started\n");

	/*
	 * Allocate driver private structure
	 */
	flash = devm_kzalloc(&spi->dev,
			     sizeof(*flash),
			     GFP_KERNEL);

	if (!flash)
		return -ENOMEM;

	/*
	 * Save SPI device pointer
	 */
	flash->spi = spi;

	
	//  Initialize driver lock
	mutex_init(&flash->lock);

	
	//  Get MTD structure inside our private structure
	 
	mtd = &flash->mtd;

	/*
	 * Configure SPI
	 */
	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;

	ret = spi_setup(spi);
	if (ret) {
		dev_err(&spi->dev,
			"spi_setup failed: %d\n",
			ret);
		return ret;
	}

	dev_info(&spi->dev,
		 "SPI configured: mode=%u bits=%u\n",
		 spi->mode,
		 spi->bits_per_word);

	/*
	 * Detect W25Q128 using JEDEC ID
	 */
	ret = w25q128_detect(spi);

	if (ret) {
		dev_err(&spi->dev,
			"W25Q128 detection failed\n");
		return ret;
	}

	/*
	 * Initialize MTD
	 */
	mtd->name = "w25q128";
	mtd->type = MTD_NORFLASH;

	/*
	 * W25Q128:
	 *
	 * 128 Mbit = 16 MiB
	 */
	mtd->size = W25Q128_FLASH_SIZE;

	/*
	 * Minimum erase size = 4 KiB
	 */
	mtd->erasesize = W25Q128_SECTOR_SIZE;

	/*
	 * Minimum write size
	 */
	mtd->writesize = 1;

	/*
	 * Maximum page program size
	 */
	mtd->writebufsize = W25Q128_PAGE_SIZE;

	/*
	 * MTD operations
	 */
	mtd->_read  = w25q128_read;
	mtd->_write = w25q128_write;
	mtd->_erase = w25q128_erase;

	/*
	 * NOR flash capability
	 */
	mtd->flags = MTD_CAP_NORFLASH;

	/*
	 * Parent device
	 */
	mtd->dev.parent = &spi->dev;

	/*
	 * Store driver private structure
	 */
	spi_set_drvdata(spi, flash);

	dev_info(&spi->dev,
		 "Registering W25Q128 MTD device\n");
/* int mtd_device_register(struct mtd_info *master,
                        const struct mtd_partition *parts,
                        int nr_parts);
*/
	ret = mtd_device_register(mtd, NULL, 0);//passing NULL, as we dont want our system to be partitioned and registered as a single block

	if (ret) {
		dev_err(&spi->dev,
			"MTD registration failed: %d\n",
			ret);
		return ret;
	}

	dev_info(&spi->dev,
		 "W25Q128 MTD registered successfully\n");

	return 0;
}

/*
 * Remove function
 */
static void w25q128_remove(struct spi_device *spi)
{
	struct w25q128 *flash;

	/*
	 * Retrieve our private structure
	 */
	flash = spi_get_drvdata(spi);

	if (flash)
		mtd_device_unregister(&flash->mtd);

	dev_info(&spi->dev,
		 "W25Q128 removed\n");
}

/*
 * Device Tree matching table
 */
 static const struct spi_device_id w25q128_id_table[] = {
    { "my-w25q128", 0 },
    { }
};

MODULE_DEVICE_TABLE(spi, w25q128_id_table);

static const struct of_device_id w25q128_of_match[] = {
    {
        .compatible = "my-spi-nor,my-w25q128",
    },
    { }
};

MODULE_DEVICE_TABLE(of, w25q128_of_match);

static struct spi_driver w25q128_driver = {
    .driver = {
        .name = "w25q128",
        .of_match_table = w25q128_of_match,
    },
    .probe = w25q128_probe,
    .remove = w25q128_remove,
    .id_table = w25q128_id_table,
};


/*
 * Register SPI driver
 */
module_spi_driver(w25q128_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gangadhar");
MODULE_DESCRIPTION("W25Q128 SPI NOR Flash MTD Driver");
