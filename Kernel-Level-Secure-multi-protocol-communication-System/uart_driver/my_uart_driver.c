#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/workqueue.h>
#include <linux/spinlock.h>
#include <linux/poll.h>

#include "../include/secure_comm_ioctl.h"

/*
 * Driver identity strings:
 * - DRIVER_NAME: used in logs/module identity
 * - DEVICE_NAME: name of /dev node (e.g., /dev/secure_uart)
 * - CLASS_NAME : sysfs class name under /sys/class
 */
#define DRIVER_NAME "my_uart_driver"
#define DEVICE_NAME "secure_uart"
#define CLASS_NAME  "secure_uart_cls"

/* Maximum payload bytes handled in one request */
#define MAX_BUF 512

/* Character-device registration objects */
static dev_t dev_num; // Device number (major/minor) allocated for our char device
static struct cdev secure_cdev; // Character device structure
static struct class *secure_class; // Device class for sysfs and udev
static struct device *secure_dev; // Device structure for /dev node creation

/*
 * rxbuf/rxlen:
 * - hold the processed output that user space will read().
 *
 * secure_mode:
 * - current operation mode selected by ioctl:
 *   ENCRYPT / DECRYPT / PASS
 */
static char rxbuf[MAX_BUF];
static int rxlen;
static int secure_mode = SECURE_MODE_PASS;

/*
 * read_wq:
 * - read() sleeps here until output becomes available.
 *
 * rx_lock:
 * - protects shared buffers and lengths used across
 *   write(), workqueue callback, and read().
 */
static DECLARE_WAIT_QUEUE_HEAD(read_wq);
static DEFINE_SPINLOCK(rx_lock);

/*
 * Workqueue:
 * - heavy/real processing is moved out of write() path
 *   into deferred context.
 */
static struct workqueue_struct *secure_wq;
static struct work_struct secure_work;

/*
 * pending_data/pending_len:
 * - latest data written by user before processing.
 */
static char pending_data[MAX_BUF];
static int pending_len;

/*
 * XOR-based demo "encryption/decryption".
 * NOTE:
 * - This is only for demonstration and not real security.
 * - For XOR, encrypt and decrypt are same operation.
 */
static void aes_like_process(char *buf, int len, int mode)
{
	int i;
	u8 key = 0x5A;

	/* Pass mode: no transformation */
	if (mode == SECURE_MODE_PASS)
		return;

	/* Encrypt/Decrypt mode: XOR each byte */
	for (i = 0; i < len; i++)
		buf[i] ^= key;
}

/*
 * Workqueue callback:
 * - takes pending input,
 * - applies transform according to secure_mode,
 * - stores result into rxbuf,
 * - wakes sleeping readers.
 */
static void secure_work_fn(struct work_struct *work)
{
	unsigned long flags;

	spin_lock_irqsave(&rx_lock, flags);

	if (pending_len > 0) {
		aes_like_process(pending_data, pending_len, secure_mode);

		memcpy(rxbuf, pending_data, pending_len);
		rxlen = pending_len;
	}

	spin_unlock_irqrestore(&rx_lock, flags);

	/* Notify read() that data is ready */
	wake_up_interruptible(&read_wq);
}

/* open(): called when user opens /dev/secure_uart */
static int secure_open(struct inode *inode, struct file *file)
{
	pr_info("secure_uart: device opened\n");
	return 0;
}

/* release(): called when user closes /dev/secure_uart */
static int secure_release(struct inode *inode, struct file *file)
{
	pr_info("secure_uart: device closed\n");
	return 0;
}

/*
 * write():
 * - copies user input into kernel pending buffer,
 * - queues work for processing,
 * - returns number of bytes accepted.
 */
static ssize_t secure_write(struct file *file,
			    const char __user *user_buffer,
			    size_t count,
			    loff_t *offset) 
{
	char kbuf[MAX_BUF]; // Temporary kernel buffer for user input
	unsigned long flags; // For spinlock protection of shared pending_data/pending_len	
	int n = min_t(size_t, count, MAX_BUF - 1);// Limit input to MAX_BUF-1 to leave space for null terminator if needed

	if (n <= 0)// No data to write or input too large
		return -EINVAL;// Invalid argument

	if (copy_from_user(kbuf, user_buffer, n))// Failed to copy data from user space
		return -EFAULT;// Bad address

	spin_lock_irqsave(&rx_lock, flags); // Protect shared pending_data/pending_len

	memcpy(pending_data, kbuf, n);// Store user input into pending_data for processing
	pending_len = n;// Set pending_len to the number of bytes to process

	spin_unlock_irqrestore(&rx_lock, flags);// Release lock after updating pending data

	/* Deferred processing */
	queue_work(secure_wq, &secure_work);// Schedule workqueue to process the pending data

	return n;// Return number of bytes accepted for processing
}

/*
 * read():
 * - blocks until processed output is available (rxlen > 0),
 * - copies output to user buffer,
 * - clears lengths for next cycle.
 */
static ssize_t secure_read(struct file *file,// Read from device
			   char __user *user_buffer,
			   size_t count,
			   loff_t *offset)
{
	unsigned long flags;// For spinlock protection of shared rxbuf/rxlen
	int n;// Number of bytes to copy to user buffer

	/* Sleep until worker publishes processed data */
	if (wait_event_interruptible(read_wq, rxlen > 0))// Wait until rxlen > 0, meaning processed data is available. If interrupted, return -ERESTARTSYS to allow signal handling.
		return -ERESTARTSYS;// Interrupted by signal, return error to allow user space to handle it

	spin_lock_irqsave(&rx_lock, flags);// Protect shared rxbuf/rxlen while copying data to user space

	n = min_t(size_t, count, rxlen);// Limit number of bytes to copy to user buffer to the smaller of count and rxlen

	if (copy_to_user(user_buffer, rxbuf, n)) {// Failed to copy data to user space
		spin_unlock_irqrestore(&rx_lock, flags);// Release lock
		return -EFAULT;// Bad address
	}

	/* Consume current processed data */
	rxlen = 0;	// Clear rxlen to indicate data has been consumed
	pending_len = 0;//	

	spin_unlock_irqrestore(&rx_lock, flags);//	Release lock after copying data to user space

	return n;// Return number of bytes read by user space
}

/*
 * ioctl():
 * - SET_MODE: set encrypt/decrypt/pass mode from user space
 * - GET_MODE: read current mode
 */
static long secure_ioctl(struct file *file,// Handle ioctl commands from user space
			 unsigned int cmd,
			 unsigned long arg)
{
	int mode;// Temporary variable to hold mode value from user space for SET_MODE command

	switch (cmd) {// Handle different ioctl commands

	case SECURE_UART_IOC_SET_MODE://	Set mode command

		if (copy_from_user(&mode,// Copy mode value from user space to kernel variable
				   (int __user *)arg,
				   sizeof(mode)))
			return -EFAULT;// Failed to copy data from user space

		if (mode < SECURE_MODE_ENCRYPT ||// Validate mode value to ensure it's within expected range
		    mode > SECURE_MODE_PASS)
			return -EINVAL;

		secure_mode = mode;// Update global secure_mode variable with the new mode

		pr_info("secure_uart: mode changed to %d\n",// Log the new mode for debugging purposes
			secure_mode);

		return 0;// Success

	case SECURE_UART_IOC_GET_MODE:// Get mode command

		if (copy_to_user((int __user *)arg,//	Copy current mode value from kernel variable to user space
				 &secure_mode,
				 sizeof(secure_mode)))
			return -EFAULT;

		return 0;// Success

	default:
		return -EINVAL;// Invalid command
	}
}

/*
 * poll():
 * - allows user space to use select/poll/epoll.
 * - reports readable when rxlen > 0.
 */
static __poll_t secure_poll(struct file *file,// Support poll/select/epoll for user space
			    poll_table *wait)
{
	__poll_t mask = 0;// Mask to indicate events that are ready (e.g., EPOLLIN for readable)

	poll_wait(file, &read_wq, wait);// Register the wait queue with the poll system so that it can wake up when data becomes available

	if (rxlen > 0)// If there is processed data available to read, set the mask to indicate that the file is readable
		mask |= EPOLLIN | EPOLLRDNORM;// Indicate that the file is ready for reading (normal data available)

	return mask;// Return the event mask to the caller
}

/* Character-device file operations table */
static const struct file_operations secure_fops = {// Define the file operations for our character device
	.owner          = THIS_MODULE,
	.open           = secure_open,
	.release        = secure_release,
	.write          = secure_write,
	.read           = secure_read,
	.poll           = secure_poll,
	.unlocked_ioctl = secure_ioctl,
};

/*
 * Module init:
 * 1) create workqueue
 * 2) allocate char-device major/minor
 * 3) add cdev
 * 4) create class and /dev node
 */
static int __init secure_uart_init(void)// Initialize the module and set up the character device
{
	int ret;// Return value for error handling

	secure_wq =
		create_singlethread_workqueue("secure_uart_wq");// Create a single-threaded workqueue for processing data asynchronously

	if (!secure_wq)// Failed to create workqueue
		return -ENOMEM;// Return error code for out of memory

	INIT_WORK(&secure_work, secure_work_fn);// Initialize the work structure with the callback function that will process the data

	ret = alloc_chrdev_region(&dev_num,//	Allocate a major number for our character device and store it in dev_num. The minor number is set to 0, and we request 1 device number for our driver.
				  0,
				  1,
				  DEVICE_NAME);

	if (ret)	// Failed to allocate character device region
		goto err_wq;// Return error code from alloc_chrdev_region

	cdev_init(&secure_cdev, &secure_fops);// Initialize the character device structure with our file operations

	ret = cdev_add(&secure_cdev, dev_num, 1);//	Add the character device to the system, making it available for use. We specify the device number and the number of devices (1 in this case).

	if (ret)// Failed to add character device to the system
		goto err_chrdev;// Return error code from cdev_add

	secure_class = class_create(CLASS_NAME);// Create a device class for our character device, which will be used to create a device node in /dev and for sysfs entries

	if (IS_ERR(secure_class)) {// Failed to create device class
		ret = PTR_ERR(secure_class);// Get the error code from the pointer returned by class_create
		goto err_cdev;//	Return error code for class creation failure
	}

	secure_dev = device_create(secure_class,// Create a device node in /dev for our character device, associating it with the class we just created. The device will be named according to DEVICE_NAME and will use the major/minor number allocated earlier.
				   NULL,
				   dev_num,
				   NULL,
				   DEVICE_NAME);

	if (IS_ERR(secure_dev)) { // Failed to create device node in /dev
		ret = PTR_ERR(secure_dev);// Get the error code from the pointer returned by device_create
		goto err_class;// Return error code for device creation failure
	}

	pr_info("secure_uart: loaded major=%d\n",// Log the major number allocated for our character device for debugging purposes
		MAJOR(dev_num));// Extract the major number from dev_num and print it

	return 0;

err_class:
	class_destroy(secure_class);// Clean up the device class if device creation failed

err_cdev:
	cdev_del(&secure_cdev);// Remove the character device from the system if adding it failed

err_chrdev:
	unregister_chrdev_region(dev_num, 1);// Unregister the character device region if allocation failed

err_wq:
	destroy_workqueue(secure_wq);// Destroy the workqueue if creation failed

	return ret;// Return the error code from the point of failure
}

/*
 * Module exit:
 * - flush and destroy workqueue,
 * - remove /dev node, class, cdev,
 * - free major/minor.
 */
static void __exit secure_uart_exit(void)// Clean up the module and remove the character device
{
	flush_workqueue(secure_wq);//	Flush the workqueue to ensure that all pending work has completed before we destroy it
	destroy_workqueue(secure_wq);// Destroy the workqueue to free resources

	device_destroy(secure_class, dev_num);// Remove the device node from /dev and clean up the device structure
	class_destroy(secure_class);// Destroy the device class to clean up sysfs entries and class structure

	cdev_del(&secure_cdev);// Remove the character device from the system to clean up the cdev structure
	unregister_chrdev_region(dev_num, 1);// Unregister the character device region to free the major/minor numbers allocated for our device

	pr_info("secure_uart: unloaded\n");// Log that the module has been unloaded for debugging purposes
}

module_init(secure_uart_init);// Specify the initialization function for the module
module_exit(secure_uart_exit);// Specify the cleanup function for the module

MODULE_LICENSE("GPL");// Declare the license for the module, which is GPL in this case. This is important for legal reasons and also affects kernel symbol availability.
MODULE_AUTHOR("Nandini + Project Integration");// Declare the author of the module for informational purposes
MODULE_DESCRIPTION("Kernel-level secure UART driver using workqueue");// Provide a brief description of the module for informational purposes
