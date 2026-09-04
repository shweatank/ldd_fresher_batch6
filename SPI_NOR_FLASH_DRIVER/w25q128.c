#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/spi/spi.h>
#include <linux/of.h>
#include <linux/mtd/mtd.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/delay.h>

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


// Send Write Enable command and set WEL bit inside flash
static int w25q128_write_enable(struct spi_device *spi)
{
    u8 cmd =W25Q128_CMD_WRITE_ENABLE ;

    return spi_write(spi, &cmd, 1);
}

// Read the flash status register
static int w25q128_read_status(struct spi_device *spi, u8 *status)
{
    u8 cmd = W25Q128_CMD_READ_STATUS;
    int ret;

    ret = spi_write_then_read(spi,&cmd,1,status,1);

    return ret;
}

// Keep checking until flash is no longer busy
static int w25q128_wait_ready(struct spi_device *spi)
{
    u8 status;
    int ret;

    do {
        ret = w25q128_read_status(spi, &status);

        if (ret)
            return ret;

        // BUSY = 1 means flash is doing an operation
        if (!(status & 0x01))
            break;

        // Wait a little before checking again
        usleep_range(1000, 2000);

    } while (1);

    return 0;
}

static int w25q128_page_program(struct w25q128 *dev,loff_t addr,const u8 *buf,size_t len)
{
    u8 *tx;
    int ret;

    // Allocate buffer for command + 3-byte address + data
    tx = kmalloc(4 + len, GFP_KERNEL);
    if (!tx)
        return -ENOMEM;

    tx[0] = W25Q128_CMD_PAGE_PROGRAM;

    // Send 24-bit flash address
    tx[1] = (addr >> 16) & 0xFF;
    tx[2] = (addr >> 8) & 0xFF;
    tx[3] = addr & 0xFF;

    // Copy user data after command and address
    memcpy(&tx[4], buf, len);

    // Send command, address and data to flash
    ret = spi_write(dev->spi, tx, 4 + len);

    kfree(tx);

    return ret;
}

static int w25q128_write(struct mtd_info *mtd,loff_t to,size_t len,size_t *retlen,const u_char *buf)
{
    struct w25q128 *dev;
    size_t written = 0;
    int ret = 0;

    // Get our private driver structure from MTD structure
    dev = container_of(mtd, struct w25q128, mtd);

    *retlen = 0;

    // Check that write address and length are inside flash
    if (to < 0 || to + len > mtd->size)
        return -EINVAL;

    // Prevent two operations from using SPI flash at the same time
    mutex_lock(&dev->lock);

    while (written < len) {

        loff_t addr = to + written;

        // Find current position inside the 256-byte page
        size_t page_off = addr % W25Q128_PAGE_SIZE;

        // Calculate how many bytes to write in on pone page
        size_t chunk = W25Q128_PAGE_SIZE - page_off;

        // Do not write more than remaining data
        if (chunk > len - written)
            chunk = len - written;

        // Enable write before programming flash
        ret = w25q128_write_enable(dev->spi);
        if (ret < 0)
            break;

        // Program one page
        ret = w25q128_page_program(dev,addr,buf + written,chunk);
        if (ret < 0)
            break;

        // Wait until page programming is complete
        ret = w25q128_wait_ready(dev->spi);
        if (ret < 0)
            break;

        written += chunk;
    }

    // Release the flash lock
    mutex_unlock(&dev->lock);

    *retlen = written;

    if (!ret)
        {
            // Write operation completed
            dev_info(&dev->spi->dev,"W25Q128 WRITE Done\n");
        }

    return ret;
}

static int w25q128_sector_erase(struct w25q128 *dev, u32 addr)
{
    u8 cmd[4];
    int ret;

    // First byte is erase command, next 3 bytes are address
    cmd[0] = W25Q128_CMD_SECTOR_ERASE;
    cmd[1] = (addr >> 16) & 0xFF;
    cmd[2] = (addr >> 8) & 0xFF;
    cmd[3] = addr & 0xFF;

    // Erase is a write operation, so enable write first
    ret = w25q128_write_enable(dev->spi);
    if (ret < 0)
        return ret;

    // Send erase command and address
    ret = spi_write(dev->spi, cmd, sizeof(cmd));
    if (ret < 0)
        return ret;

    // Wait until sector erase is completed
    ret = w25q128_wait_ready(dev->spi);

    return ret;
}

static int w25q128_erase(struct mtd_info *mtd,struct erase_info *instr)
{
    struct w25q128 *dev = container_of(mtd, struct w25q128, mtd);

    u32 addr = instr->addr;
    u32 len = instr->len;
    int ret;

    // Erase must start and end on a sector boundary
    if (addr % mtd->erasesize || len % mtd->erasesize)
        return -EINVAL;

    // Check that erase range is inside flash
    if ((u64)addr + len > mtd->size)
        return -EINVAL;

    // Prevent other flash operations during erase
    mutex_lock(&dev->lock);

    while (len) {

        // Erase one 4-KB sector
        ret = w25q128_sector_erase(dev, addr);

        if (ret < 0) {
            instr->fail_addr = addr;
            mutex_unlock(&dev->lock);
            return ret;
        }

        // Move to next sector
        addr += mtd->erasesize;
        len -= mtd->erasesize;
    }

    mutex_unlock(&dev->lock);

    dev_info(&dev->spi->dev,
         "SPI NOR ERASE Done\n");

    return 0;
}


// Read the JEDEC ID to check whether the connected flash is W25Q128
static int w25q128_detect(struct spi_device *spi)
{
	u8 cmd = W25Q128_CMD_JEDEC_ID;
	u8 id[3];
	int ret;

    // Send 0x9F and receive 3 ID bytes
	ret = spi_write_then_read(spi, &cmd, 1, id, 3);

	if (ret) {
		dev_err(&spi->dev,"JEDEC ID read failed: %d\n", ret);
		return ret;
	}

	dev_info(&spi->dev, "JEDEC ID: %02X %02X %02X\n",id[0], id[1], id[2]);

	// Check manufacturer, memory type and capacity bytes
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


// MTD read function called when Linux wants data from flash
static int w25q128_read(struct mtd_info *mtd, loff_t from, size_t len, size_t *retlen,u_char *buf)
{
    struct w25q128 *flash;
    u8 cmd[4];
    int ret;

    // Get our private structure from MTD structure
    flash = container_of(mtd, struct w25q128, mtd);

    dev_info(&flash->spi->dev,
             "READ called: from=0x%llx len=%zu\n",
             from, len);

    // Check that read starts inside flash
    if (from >= mtd->size)
        return -EINVAL;

    // Check that complete read range is inside flash
    if (len > mtd->size - from)
        return -EINVAL;

    // First byte is read command
    cmd[0] = W25Q128_CMD_READ;

    // Next 3 bytes are 24-bit flash address
    cmd[1] = (from >> 16) & 0xff;
    cmd[2] = (from >> 8) & 0xff;
    cmd[3] = from & 0xff;

    // Protect SPI flash access
    mutex_lock(&flash->lock);

    // Send command + address and receive flash data
    ret = spi_write_then_read(flash->spi,cmd,4,buf,len);

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

	// Allocate memory for our driver structure
	flash = devm_kzalloc(&spi->dev,sizeof(*flash),GFP_KERNEL);

	if (!flash)
		return -ENOMEM;

	// Save SPI device pointer
	flash->spi = spi;

	// Initialize lock used to protect flash operations
	mutex_init(&flash->lock);

	// Get MTD structure from our driver structure
	mtd = &flash->mtd;

	// Configure SPI mode and data size
	spi->mode = SPI_MODE_0;
	spi->bits_per_word = 8;

    // Apply SPI configuration to SPI controller
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

	// Check whether connected chip is W25Q128
	ret = w25q128_detect(spi);

	if (ret) {
		dev_err(&spi->dev,
			"W25Q128 detection failed\n");
		return ret;
	}

	// Fill MTD information for this flash
	mtd->name = "w25q128";
	mtd->type = MTD_NORFLASH;

	// W25Q128 = 128 Mbit = 16 MB
	mtd->size = W25Q128_FLASH_SIZE;

	// Flash can erase one sector of 4 KB
	mtd->erasesize = W25Q128_SECTOR_SIZE;

	// Minimum write unit is 1 byte
	mtd->writesize = 1;

	// Maximum data sent in one page program
	mtd->writebufsize = W25Q128_PAGE_SIZE;

	// Connect our read, write and erase functions to MTD
	mtd->_read  = w25q128_read;
	mtd->_write = w25q128_write;
	mtd->_erase = w25q128_erase;

	// Tell MTD that this is a NOR flash
	mtd->flags = MTD_CAP_NORFLASH;

	// Make MTD device a child of SPI device
	mtd->dev.parent = &spi->dev;

	// Store our private structure for later use
	spi_set_drvdata(spi, flash);

	dev_info(&spi->dev,
		 "Registering W25Q128 MTD device\n");

	// Register flash with Linux MTD framework
	ret = mtd_device_register(mtd, NULL, 0);

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


// Called when SPI device/driver is removed
static void w25q128_remove(struct spi_device *spi)
{
	struct w25q128 *flash;

	// Get our private structure back
	flash = spi_get_drvdata(spi);

	if (flash)
		// Remove MTD device from Linux
		mtd_device_unregister(&flash->mtd);

	dev_info(&spi->dev,
		 "W25Q128 removed\n");
}


// SPI device name used for non-DT matching
static const struct spi_device_id w25q128_id_table[] = {
    { "my-w25q128", 0 },
    { }
};

MODULE_DEVICE_TABLE(spi, w25q128_id_table);


// Device Tree compatible string
static const struct of_device_id w25q128_of_match[] = {
    {
        .compatible = "my-spi-nor,my-w25q128",
    },
    { }
};

MODULE_DEVICE_TABLE(of, w25q128_of_match);


// Connect probe, remove and Device Tree matching functions
static struct spi_driver w25q128_driver = {
    .driver = {
        .name = "w25q128",
        .of_match_table = w25q128_of_match,
    },
    .probe = w25q128_probe,
    .remove = w25q128_remove,
    .id_table = w25q128_id_table,
};


// Register this driver with Linux SPI framework
module_spi_driver(w25q128_driver);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SPI_NOR_FLASH_DRIVER");
MODULE_DESCRIPTION("W25Q128 SPI NOR Flash MTD Driver");