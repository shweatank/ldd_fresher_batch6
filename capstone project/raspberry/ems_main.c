#include <linux/module.h>   // Core kernel module definitions
#include <linux/kernel.h>   // Required for printk()
#include <linux/init.h>     // Required for __init and __exit macros
#include <linux/fs.h>       // Character device registration functions
#include <linux/uaccess.h>  // copy_to_user() and copy_from_user()
#include <linux/device.h>   // Device class creation functions

#include "dht11.h"          // DHT22 sensor driver header
#include "ssd1306_oled_i2c.h" // OLED display driver header

// Name of the character device
#define DEVICE_NAME "thd_driver"


// Pointer for device class
static struct class *thd_class;

// Variable to store dynamically assigned major number
static int major;


/* -------- Structure for Temperature & Humidity Data -------- */

// Structure used to send sensor data to user space
struct temp_hum_t
{
	int temp;   // Stores temperature value
	int hum;    // Stores humidity value
};

// Global structure instance
struct temp_hum_t th_data;


/* -------- Device Open Function -------- */

// Called when device file is opened
static int thd_open(struct inode *inode, struct file *file)
{
        // Print message in kernel log
        printk(KERN_INFO "%s: device opened\n", DEVICE_NAME);

        return 0;
}


/* -------- Device Release Function -------- */

// Called when device file is closed
static int thd_release(struct inode *inode, struct file *file)
{
        // Print message in kernel log
        printk(KERN_INFO "basic_char: device closed");

        return 0;
}


/* -------- Device Read Function -------- */

// Called when user reads from device file
static ssize_t thd_read(struct file *file,
                        char __user *user_buffer,
                        size_t count,
                        loff_t *offset)
{
        // Read temperature and humidity from DHT22 sensor
	int ret = dht11_read_data(&th_data.temp, &th_data.hum);

        // Display sensor values on OLED screen
	oled_display_sensor(th_data.temp, th_data.hum, ret);

        // Copy sensor data from kernel space to user space
	if(copy_to_user(user_buffer, &th_data, sizeof(th_data)))
                return -EFAULT;

        // Print read information in kernel log
        printk(KERN_INFO "%s : read %ld bytes \n",
               DEVICE_NAME,
               sizeof(th_data));

        // Return number of bytes read
        return sizeof(th_data);
}


/* -------- File Operations Structure -------- */

// Connects system calls to driver functions
static struct file_operations thd_fops = {

  .owner = THIS_MODULE,     // Owner of this module
  .open = thd_open,         // Open function
  .read = thd_read,         // Read function
  .release = thd_release,   // Close function
};


/* -------- Module Initialization Function -------- */

// Called when module is inserted using insmod
static int __init thd_init(void)
{
        // Register character device
        major = register_chrdev(0, DEVICE_NAME, &thd_fops);

        // Create device class
        thd_class = class_create(DEVICE_NAME);

        // Create device file in /dev/
        device_create(thd_class,
                      NULL,
                      MKDEV(major,0),
                      NULL,
                      DEVICE_NAME);

        // Initialize DHT22 sensor driver
        dht11_init();

        // Initialize OLED display driver
	oled_init();

        // Print module loaded message
        printk(KERN_INFO "%s : module loaded\n", DEVICE_NAME);

        // Returning 0 means successful loading
        return 0;
}


/* -------- Module Exit Function -------- */

// Called when module is removed using rmmod
static void __exit thd_exit(void)
{
        // Remove device file
	device_destroy(thd_class, MKDEV(major,0));

        // Destroy device class
        class_destroy(thd_class);

        // Unregister character device
        unregister_chrdev(major, DEVICE_NAME);

        // Release DHT22 GPIO resources
	dht11_exit();

        // Release OLED resources
	oled_exit();

        // Print module unloaded message
        printk(KERN_INFO "%s: unloaded\n", DEVICE_NAME);
}


/* -------- Register Module Entry & Exit -------- */

// Register initialization function
module_init(thd_init);

// Register cleanup function
module_exit(thd_exit);


/* -------- Module Information -------- */

MODULE_LICENSE("GPL");   // Module license
MODULE_AUTHOR("Almas");  // Author name
MODULE_VERSION("1.0");   // Driver version
MODULE_DESCRIPTION("Temperature and Humidity Monitoring Using DHT22");
