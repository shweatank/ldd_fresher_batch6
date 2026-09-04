#include <linux/module.h>      // Kernel module
#include <linux/kernel.h>     // Kernel functions
#include <linux/init.h>       // module_init/module_exit
#include <linux/usb.h>         // USB functions
#include <linux/slab.h>         // kmalloc/kzalloc/kfree
#include <linux/errno.h>        // Error codes
#include <linux/types.h>        // u8, u32, u64
#include <linux/byteorder/generic.h> // Endian conversion
#include <linux/unaligned.h>    // Unaligned data access
#include <linux/string.h>       // memcpy
#include <linux/blkdev.h>       // Block device
#include <linux/blk-mq.h>       // Block request handling
#include <linux/bio.h>          // BIO structures
#include <linux/highmem.h>      // Page mapping
#include <linux/mutex.h>        // Mutex
#include <linux/atomic.h>       // Atomic variables

#define USB_SUBCLASS_SCSI        0x06
#define USB_PROTOCOL_BULK_ONLY   0x50

#define CBW_SIGNATURE            0x43425355U
#define CSW_SIGNATURE            0x53425355U
#define CBW_SIZE                 31
#define CSW_SIZE                 13

#define CBW_FLAG_OUT             0x00
#define CBW_FLAG_IN              0x80

#define CSW_STATUS_PASS          0x00
#define CSW_STATUS_FAIL          0x01
#define CSW_STATUS_PHASE         0x02

#define USB_BULK_RESET_REQUEST   0xFF
#define USB_MS_TIMEOUT           5000

#define SCSI_TEST_UNIT_READY     0x00
#define SCSI_REQUEST_SENSE       0x03
#define SCSI_INQUIRY             0x12
#define SCSI_READ_CAPACITY10     0x25
#define SCSI_READ10              0x28
#define SCSI_WRITE10             0x2A

#define SCSI_INQUIRY_LENGTH      36
#define SCSI_SENSE_LENGTH        18
#define SCSI_READ_CAPACITY_LEN   8

#define USB_MS_QUEUE_DEPTH       1   
#define USB_MS_MAX_TRANSFER      128
#define USB_MS_MAX_DEVICES       16
#define USB_MS_MINORS            16

static int major_number;
static atomic_t dev_index = ATOMIC_INIT(0);

struct cbw {
	__le32 signature;
	__le32 tag;
	__le32 data_transfer_length;
	u8 flags;
	u8 lun;
	u8 cb_length;
	u8 cb[16];
} __packed;

struct csw {
	__le32 signature;
	__le32 tag;
	__le32 data_residue;
	u8 status;
} __packed;

struct my_usb{
	struct usb_device *udev;
	struct usb_interface *interface;

	u8 bulk_in;
	u8 bulk_out;

	u32 tag;			//command number
	u32 last_lba;		//last storage block
	u32 block_size;		//size of one block
	u64 capacity;		// total storage size

	struct gendisk *disk;
	struct blk_mq_tag_set tag_set;

	int index;
	struct mutex io_mutex;   //Protect USB read/write operations
	atomic_t disconnected;
};

static int usb_mass_storage_reset_recovery(struct my_usb*dev)
{
	int ret;

	ret = usb_control_msg(
		dev->udev,
		usb_sndctrlpipe(dev->udev, 0),
		USB_BULK_RESET_REQUEST,
		USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0,
		dev->interface->cur_altsetting->desc.bInterfaceNumber,
		NULL,
		0,
		USB_MS_TIMEOUT
	);

	if (ret < 0)
		dev_err(&dev->interface->dev, "Mass storage reset failed: %d\n", ret);

	usb_clear_halt(dev->udev,
		       usb_sndbulkpipe(dev->udev, dev->bulk_out));

	usb_clear_halt(dev->udev,
		       usb_rcvbulkpipe(dev->udev, dev->bulk_in));

	return ret;
}

static int submit_cbw(struct my_usb*dev, struct cbw *cbw)
{
	unsigned int pipe;
	int transferred;
	int ret;

	pipe = usb_sndbulkpipe(dev->udev, dev->bulk_out); //This creates an OUT bulk pipe.

	ret = usb_bulk_msg(dev->udev, pipe, cbw, CBW_SIZE,
			   &transferred, USB_MS_TIMEOUT);

	if (ret == -EPIPE) {
		usb_clear_halt(dev->udev, pipe);
		return -EIO;
	}

	if (ret)
		return ret;

	if (transferred != CBW_SIZE)
		return -EIO;

	return 0;
}

static int submit_data_in(struct my_usb*dev, void *buffer, int length)
{
	unsigned int pipe;
	int transferred;
	int ret;

	pipe = usb_rcvbulkpipe(dev->udev, dev->bulk_in);

	ret = usb_bulk_msg(dev->udev, pipe, buffer, length,
			   &transferred, USB_MS_TIMEOUT);

	if (ret == -EPIPE) {
		usb_clear_halt(dev->udev, pipe);
		return -EIO;
	}

	if (ret)
		return ret;

	return transferred;
}

static int submit_data_out(struct my_usb*dev,
			   const void *buffer, int length)
{
	unsigned int pipe;
	int transferred;
	int ret;

	pipe = usb_sndbulkpipe(dev->udev, dev->bulk_out);

	ret = usb_bulk_msg(dev->udev, pipe, (void *)buffer, length,
			   &transferred, USB_MS_TIMEOUT);

	if (ret == -EPIPE) {
		usb_clear_halt(dev->udev, pipe);
		return -EIO;
	}

	if (ret)
		return ret;

	if (transferred != length)
		return -EIO;

	return 0;
}

static int submit_csw(struct my_usb*dev,
			  struct csw *csw, u32 expected_tag)
{
	unsigned int pipe;
	u32 signature;
	u32 tag;
	u32 residue;
	u8 status;
	int transferred;
	int ret;

	pipe = usb_rcvbulkpipe(dev->udev, dev->bulk_in);

	ret = usb_bulk_msg(dev->udev, pipe, csw, CSW_SIZE,
			   &transferred, USB_MS_TIMEOUT);

	if (ret == -EPIPE) {
		usb_clear_halt(dev->udev, pipe);

		ret = usb_bulk_msg(dev->udev, pipe, csw, CSW_SIZE,
				   &transferred, USB_MS_TIMEOUT);
	}

	if (ret) {
		usb_mass_storage_reset_recovery(dev);
		return ret;
	}

	if (transferred != CSW_SIZE)
		return -EIO;

	signature = le32_to_cpu(csw->signature);
	tag = le32_to_cpu(csw->tag);
	residue = le32_to_cpu(csw->data_residue);
	status = csw->status;

	dev_info(&dev->interface->dev,"CSW signature: 0x%08x\n", signature);
	dev_info(&dev->interface->dev,"CSW tag: %u\n", tag);
	dev_info(&dev->interface->dev,"CSW residue: %u\n", residue);
	dev_info(&dev->interface->dev,"CSW status: 0x%02x\n", status);

	if (signature != CSW_SIGNATURE)
		return -EIO;

	if (tag != expected_tag)
		return -EIO;

	switch (status) {
	case CSW_STATUS_PASS:
		return 0;

	case CSW_STATUS_FAIL:
		return -EIO;

	case CSW_STATUS_PHASE:
		usb_mass_storage_reset_recovery(dev);
		return -EPROTO;

	default:
		return -EIO;
	}
}

static int build_scsi_command(struct my_usb *dev,const u8 *cdb, u8 cdb_len,void *data, u32 data_len, bool data_in)
{
	struct cbw *cbw;
	struct csw *csw;
	u32 tag;
	int transferred;
	int ret;

	if (!cdb || !cdb_len || cdb_len > 16)
		return -EINVAL;

	if (data_len && !data)
		return -EINVAL;

	cbw = kzalloc(sizeof(*cbw), GFP_KERNEL);
	if (!cbw)
		return -ENOMEM;

	csw = kzalloc(sizeof(*csw), GFP_KERNEL);
	if (!csw) {
		kfree(cbw);
		return -ENOMEM;
	}

	tag = dev->tag++;
	if (!dev->tag)
		dev->tag = 1;

	cbw->signature = cpu_to_le32(CBW_SIGNATURE);
	cbw->tag = cpu_to_le32(tag);
	cbw->data_transfer_length = cpu_to_le32(data_len);
	cbw->flags = data_in ? CBW_FLAG_IN : CBW_FLAG_OUT;
	cbw->lun = 0;
	cbw->cb_length = cdb_len;

	memcpy(cbw->cb, cdb, cdb_len);

	ret = submit_cbw(dev, cbw);
	if (ret)
		goto out;

	if (data_len) {
		if (data_in) {
			transferred = submit_data_in(dev, data, data_len);

			if (transferred < 0) {
				ret = transferred;
				goto out;
			}

			if (transferred != data_len) {
				ret = -EIO;
				goto out;
			}
		} else {
			ret = submit_data_out(dev, data, data_len);

			if (ret)
				goto out;
		}
	}

	ret = submit_csw(dev, csw, tag);

out:
	kfree(csw);
	kfree(cbw);

	return ret;
}

static int scsi_cmd_inquiry(struct my_usb*dev)
{
	u8 cdb[6] = {0};
	u8 *data;
	int ret;

	data = kzalloc(SCSI_INQUIRY_LENGTH, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	cdb[0] = SCSI_INQUIRY;
	cdb[4] = SCSI_INQUIRY_LENGTH;

	ret = build_scsi_command(dev, cdb, sizeof(cdb),
				  data, SCSI_INQUIRY_LENGTH, true);

	if (!ret) {
		dev_info(&dev->interface->dev, "INQUIRY successful\n");
		dev_info(&dev->interface->dev, "Vendor   : %.8s\n", &data[8]);
		dev_info(&dev->interface->dev, "Product  : %.16s\n", &data[16]);
		dev_info(&dev->interface->dev, "Revision : %.4s\n", &data[32]);
	}

	kfree(data);

	return ret;
}

static int scsi_cmd_test_unit_ready(struct my_usb*dev)
{
	u8 cdb[6] = {0};

	cdb[0] = SCSI_TEST_UNIT_READY;

	return build_scsi_command(dev, cdb, sizeof(cdb),NULL, 0, false);
}

static int usb_mass_storage_request_sense(struct my_usb *dev)
{
	u8 cdb[6] = {0};
	u8 *sense;
	int ret;

	sense = kzalloc(SCSI_SENSE_LENGTH, GFP_KERNEL);
	if (!sense)
		return -ENOMEM;

	cdb[0] = SCSI_REQUEST_SENSE;
	cdb[4] = SCSI_SENSE_LENGTH;

	ret = build_scsi_command(dev, cdb, sizeof(cdb),sense, SCSI_SENSE_LENGTH, true);

	if (!ret)
		dev_info(&dev->interface->dev,
			 "REQUEST SENSE successful\n");

	kfree(sense);

	return ret;
}

static int scsi_cmd_read_capacity(struct my_usb*dev)
{
	u8 cdb[10] = {0};
	u8 *data;
	u32 last_lba;
	u32 block_size;
	u64 blocks;
	u64 capacity;
	int ret;

	data = kzalloc(SCSI_READ_CAPACITY_LEN, GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	cdb[0] = SCSI_READ_CAPACITY10;

	ret = build_scsi_command(dev, cdb, sizeof(cdb),
				  data, SCSI_READ_CAPACITY_LEN, true);
	if (ret)
		goto out;

	last_lba = get_unaligned_be32(&data[0]);
	block_size = get_unaligned_be32(&data[4]);

	if (!block_size) {
		ret = -EIO;
		goto out;
	}

	blocks = (u64)last_lba + 1;
	capacity = blocks * block_size;

	dev->last_lba = last_lba;
	dev->block_size = block_size;
	dev->capacity = capacity;

	dev_info(&dev->interface->dev,
		 "Last LBA: %u\n", last_lba);
	dev_info(&dev->interface->dev,
		 "Block size: %u\n", block_size);
	dev_info(&dev->interface->dev,
		 "Capacity: %llu bytes\n", capacity);

out:
	kfree(data);

	return ret;
}

static int scsi_cmd_read10(struct my_usb*dev,u32 lba, u16 blocks, u8 *buffer)
{
	u8 cdb[10] = {0};
	u32 transfer_bytes;

	if (!buffer || !blocks || !dev->block_size)
		return -EINVAL;

	if ((u64)lba + blocks > (u64)dev->last_lba + 1)
		return -EINVAL;

	if ((u64)blocks * dev->block_size > U32_MAX)
		return -EINVAL;

	transfer_bytes = blocks * dev->block_size;

	cdb[0] = SCSI_READ10;
	cdb[2] = (lba >> 24) & 0xff;
	cdb[3] = (lba >> 16) & 0xff;
	cdb[4] = (lba >> 8) & 0xff;
	cdb[5] = lba & 0xff;
	cdb[7] = (blocks >> 8) & 0xff;
	cdb[8] = blocks & 0xff;

	return build_scsi_command(dev, cdb, sizeof(cdb),buffer, transfer_bytes, true);
}

static int scsi_cmd_write10(struct my_usb*dev,
			   u32 lba, u16 blocks, const u8 *buffer)
{
	u8 cdb[10] = {0};
	u32 transfer_bytes;

	if (!buffer || !blocks || !dev->block_size)
		return -EINVAL;

	if ((u64)lba + blocks > (u64)dev->last_lba + 1)
		return -EINVAL;

	if ((u64)blocks * dev->block_size > U32_MAX)
		return -EINVAL;

	transfer_bytes = blocks * dev->block_size;

	cdb[0] = SCSI_WRITE10;
	cdb[2] = (lba >> 24) & 0xff;
	cdb[3] = (lba >> 16) & 0xff;
	cdb[4] = (lba >> 8) & 0xff;
	cdb[5] = lba & 0xff;
	cdb[7] = (blocks >> 8) & 0xff;
	cdb[8] = blocks & 0xff;
	return build_scsi_command(dev, cdb, sizeof(cdb),(void *)buffer, transfer_bytes, false);
}

static void copy_from_request(struct request *rq, void *buffer)
{
	struct bio_vec bvec;
	/*
  bv_page   -Which memory page
 bv_offset  - Where inside that page
  bv_len     - How many bytes
  */
	struct req_iterator iter;
	u8 *dst = buffer;
//go though every memory segment of this request one by one.
	rq_for_each_segment(bvec, rq, iter) //This is macro converted as loop
	{
		
		void *page = kmap_local_page(bvec.bv_page); //maps the page so kernel can access it.

		memcpy(dst, page + bvec.bv_offset, bvec.bv_len);

		kunmap_local(page);//removing the maping
		dst += bvec.bv_len;
	}
}

static void copy_to_request(struct request *rq, const void *buffer)
{
	struct bio_vec bvec;
		/*
  bv_page   -Which memory page
 bv_offset  - Where inside that page
  bv_len     - How many bytes
  */
	struct req_iterator iter;
	const u8 *src = buffer;
//go though every memory segment of this request one by one.
	rq_for_each_segment(bvec, rq, iter) {
		void *page = kmap_local_page(bvec.bv_page); //maps the page so kernel can access it.

		memcpy(page + bvec.bv_offset, src, bvec.bv_len);

		kunmap_local(page); //removing the mapping
		src += bvec.bv_len;
	}
}

static int usb_ms_open(struct gendisk *disk, blk_mode_t mode)
{
	struct my_usb *dev;

	dev = disk->private_data;

	if (!dev || atomic_read(&dev->disconnected))
		return -ENODEV;

	return 0;
}

static const struct block_device_operations usb_ms_fops = {
	.owner = THIS_MODULE,
	.open = usb_ms_open,
};

static blk_status_t usb_ms_queue_rq(struct blk_mq_hw_ctx *hctx,const struct blk_mq_queue_data *bd)
{
	struct request *rq = bd->rq;
	struct my_usb *dev = hctx->queue->queuedata;
	u64 sector;
	u32 lba;
	u32 blocks;
	u32 bytes;
	void *buffer;
	blk_status_t status = BLK_STS_OK;
	int ret = 0;

	blk_mq_start_request(rq);

	if (!dev || atomic_read(&dev->disconnected)) {
		status = BLK_STS_IOERR;
		goto done;
	}

	switch (req_op(rq)) {
	case REQ_OP_READ:
	case REQ_OP_WRITE:
		break;

	case REQ_OP_FLUSH:
		goto done;

	default:
		status = BLK_STS_NOTSUPP;
		goto done;
	}

	sector = blk_rq_pos(rq);
	bytes = blk_rq_bytes(rq);

	if (!dev->block_size || bytes % dev->block_size) {
		status = BLK_STS_IOERR;
		goto done;
	}

	lba = div_u64(sector * 512, dev->block_size);
	blocks = bytes / dev->block_size;

	if (!blocks || blocks > USB_MS_MAX_TRANSFER) {
		status = BLK_STS_IOERR;
		goto done;
	}

	if ((u64)lba + blocks > (u64)dev->last_lba + 1) {
		status = BLK_STS_IOERR;
		goto done;
	}

	buffer = kmalloc(bytes, GFP_KERNEL);
	if (!buffer) {
		status = BLK_STS_RESOURCE;
		goto done;
	}

	mutex_lock(&dev->io_mutex);

	if (req_op(rq) == REQ_OP_WRITE) {
		copy_from_request(rq, buffer);

		ret = scsi_cmd_write10(dev, lba, blocks, buffer);
	} else {
		ret = scsi_cmd_read10(dev, lba, blocks, buffer);

		if (!ret)
			copy_to_request(rq, buffer);
	}

	mutex_unlock(&dev->io_mutex);

	kfree(buffer);

	if (ret)
		status = BLK_STS_IOERR;

done:
	blk_mq_end_request(rq, status);

	return BLK_STS_OK;
}

static const struct blk_mq_ops usb_ms_mq_ops = {
	.queue_rq = usb_ms_queue_rq,
};

static int  usb_mass_storage_setup_block_device(struct my_usb *dev)
{
	struct queue_limits limits = {0};
	u64 sectors;
	int ret;

	dev->index = atomic_inc_return(&dev_index) - 1;

	if (dev->index >= USB_MS_MAX_DEVICES) {
		atomic_dec(&dev_index);
		return -ENOSPC;
	}

	limits.logical_block_size = dev->block_size;
	limits.physical_block_size = dev->block_size;
	limits.max_hw_sectors =(USB_MS_MAX_TRANSFER * dev->block_size) >> 9;

	dev->tag_set.ops = &usb_ms_mq_ops;          // Block I/O operations
 	dev->tag_set.nr_hw_queues = 1;              // Use 1 hardware queue
	dev->tag_set.queue_depth = USB_MS_QUEUE_DEPTH; // Max requests in queue
	dev->tag_set.numa_node = NUMA_NO_NODE;      // No specific NUMA node
	dev->tag_set.flags = BLK_MQ_F_BLOCKING;     // Request can sleep/wait

	ret = blk_mq_alloc_tag_set(&dev->tag_set);
	if (ret)
		return ret;

	dev->disk = blk_mq_alloc_disk(&dev->tag_set, &limits, dev);
	if (IS_ERR(dev->disk)) {
		ret = PTR_ERR(dev->disk);
		dev->disk = NULL;
		goto free_tags;
	}

	dev->disk->major = major_number;
	dev->disk->first_minor = dev->index * USB_MS_MINORS;
	dev->disk->minors = USB_MS_MINORS;
	dev->disk->fops = &usb_ms_fops;
	dev->disk->private_data = dev;

	snprintf(dev->disk->disk_name,
		 sizeof(dev->disk->disk_name),
		 "myusb%d", dev->index);

	sectors = div_u64((u64)(dev->last_lba + 1) *
			  dev->block_size, 512);

	set_capacity(dev->disk, sectors);

	ret = device_add_disk(&dev->interface->dev, dev->disk, NULL);
	if (ret)
		goto free_disk;

	dev_info(&dev->interface->dev,
		 "Block device /dev/%s created\n",
		 dev->disk->disk_name);

	return 0;

free_disk:
	put_disk(dev->disk);
	dev->disk = NULL;

free_tags:
	blk_mq_free_tag_set(&dev->tag_set);

	return ret;
}

static void usb_ms_remove_disk(struct my_usb*dev)
{
	if (!dev->disk)
		return;

	del_gendisk(dev->disk);
	put_disk(dev->disk);

	blk_mq_free_tag_set(&dev->tag_set);

	dev->disk = NULL;
}

static int endpoints_discovery(struct usb_interface *interface,
				 struct my_usb*dev)
{
	struct usb_host_interface *altsetting;
	struct usb_endpoint_descriptor *endpoint;
	int i;

	altsetting = interface->cur_altsetting;

	if (!altsetting)
		return -ENODEV;

	for (i = 0; i < altsetting->desc.bNumEndpoints; i++) {
		endpoint = &altsetting->endpoint[i].desc;

		if (usb_endpoint_is_bulk_in(endpoint))
			dev->bulk_in = endpoint->bEndpointAddress;

		if (usb_endpoint_is_bulk_out(endpoint))
			dev->bulk_out = endpoint->bEndpointAddress;
	}

	if (!dev->bulk_in || !dev->bulk_out)
		return -ENODEV;

	dev_info(&interface->dev,
		 "Bulk IN: 0x%02x\n", dev->bulk_in);
	dev_info(&interface->dev,
		 "Bulk OUT: 0x%02x\n", dev->bulk_out);

	return 0;
}

static int usb_mass_storage_probe(struct usb_interface *interface,
			const struct usb_device_id *id)
{
	struct usb_device *udev;
	struct my_usb *dev;
	int ret;
	//converts a USB interface pointer into the USB device pointer that owns that interface
	udev = interface_to_usbdev(interface);  

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);
	if (!dev)
		return -ENOMEM;

	dev->udev = usb_get_dev(udev);
	dev->interface = interface;
	dev->tag = 1;

	mutex_init(&dev->io_mutex);
	atomic_set(&dev->disconnected, 0);

	ret = endpoints_discovery(interface, dev);
	if (ret)
		goto error;

	usb_set_intfdata(interface, dev);

	ret = scsi_cmd_inquiry(dev);
	if (ret)
		goto error_intf;

	ret = scsi_cmd_test_unit_ready(dev);
	if (ret) {
		usb_mass_storage_request_sense(dev);
		goto error_intf;
	}

	ret = scsi_cmd_read_capacity(dev);
	if (ret)
		goto error_intf;

	ret =  usb_mass_storage_setup_block_device(dev);
	if (ret)
		goto error_intf;

	dev_info(&interface->dev,
		 "USB Mass Storage driver ready\n");

	return 0;

error_intf:
	usb_set_intfdata(interface, NULL);

error:
	usb_put_dev(dev->udev);
	kfree(dev);

	return ret;
}

static void usb_mass_storage_disconnect(struct usb_interface *interface)
{
	struct my_usb*dev;

	dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);

	if (!dev)
		return;

	atomic_set(&dev->disconnected, 1);

	mutex_lock(&dev->io_mutex);
	usb_ms_remove_disk(dev);
	mutex_unlock(&dev->io_mutex);

	usb_put_dev(dev->udev);
	kfree(dev);
}

static const struct usb_device_id usb_mass_storage_table[] = {
	{
		USB_INTERFACE_INFO(
			USB_CLASS_MASS_STORAGE,
			USB_SUBCLASS_SCSI,
			USB_PROTOCOL_BULK_ONLY
		)
	},
	{}
};

MODULE_DEVICE_TABLE(usb, usb_mass_storage_table);

static struct usb_driver usb_mass_storage_driver = {
	.name = "my_usb_mass_storage",
	.id_table = usb_mass_storage_table,
	.probe = usb_mass_storage_probe,
	.disconnect = usb_mass_storage_disconnect,
};

static int __init usb_mass_storage_init(void)
{
	int ret;

	if (sizeof(struct cbw) != CBW_SIZE ||sizeof(struct csw) != CSW_SIZE)
		return -EINVAL;

	ret = register_blkdev(0, "usb_ms");
	if (ret < 0)
		return ret;

	major_number = ret;

	ret = usb_register(&usb_mass_storage_driver);
	if (ret) {
		unregister_blkdev(major_number, "usb_ms");
		return ret;
	}

	pr_info("MY_USB: driver loaded\n");

	return 0;
}

static void __exit usb_mass_storage_exit(void)
{
	usb_deregister(&usb_mass_storage_driver);
	unregister_blkdev(major_number, "usb_ms");

	pr_info("MY_USB: driver unloaded\n");
}

module_init(usb_mass_storage_init);
module_exit(usb_mass_storage_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("P1 Team 1");
MODULE_DESCRIPTION("USB Mass Storage Device Driver");
