#include "gpio_and_uart_driver.h"

/* Character device structures for /dev/ node */
static dev_t dev_num;
static struct class *ir_class;
static struct cdev ir_cdev;

/*
UART PINS : 
    TXD2: GPIO 0 (Physical Pin 27)
    RXD2: GPIO 1 (Physical Pin 28)
*/
//PL011 base address(raspi 4)
// #define UART_BASE_PHYS 0XFE201400
// #define UART_SIZE 0x1000


// Flag register bits
// #define TXFF (1 << 5)   // Transmit FIFO Full => bit to check whether the tx fifo is full or not
// #define RXFE (1 << 4)   // Receive FIFO Empty => bit to check the FIFO is empty or not



// Control register bits for UART control
#define UARTEN (1 << 0)   //UART enable bit 
#define TXE    (1 << 8)   //UART tx enable bit
#define RXE    (1 << 9)   //UART rx enable bit


// struct tty_struct *my_tty = NULL;  //this is a pointer that can point to our UART device UART2

#define RXIM (1 << 4)


// #define GPIO_IR_SENSOR (512 +  17)  //GPIO 17 is the gpio pin that triggers the ir sensor
//GPIO 17 => PIN 11
//we are getting the interrupt(transition of gpio 17 from 0 to 1) ,i.e , when some object is detected in the sensor


//a variable to recv the uart interrupt data
static char uart_interrupt_rx_data;

//taking a flag variable to count the number of permissions given after an object gets detected
static int permission_count = 0;

//ioremap pointer to store the mapped gpio address
void __iomem *gpio_addr;  //special pointer
//iomem tells the compiler that this memory belongs to a hardware devic => I/O device

//ioremap the base physical address
static void __iomem *uart_base;
//buffer to get the buffer from user and store in kernel space
static char kbuffer[300];
char rxbuff[256];


//function to recv char by char
static char uart_recv_char(void);

//function to send string to uart to be transmitted
static void uart_send_string(char *str);

//function to send the char via UART
static void uart_send_char(char c);


static int k=1;  //a variable to check whether the interrupt triggered belongs to the uart or to the gpio sensor
static int ir_irq_number;  //variable to store the irq number after requesting for the interrupt using request_irq api
static int motion_count = 0; // Simple data to collectthe number of times the detection happens
static int pid=0;  // a variable to store the pid of the user process  through IOCTL

//creating the structure variable for my tasklet function 
struct tasklet_struct ir_tasklet;

//this tasklet for triggering the userspace pgm after receiving data through uart interrupt
struct tasklet_struct display_tasklet;

/* Function to send a single character by polling the Flag Register */
static void uart_send_char(char c) {
    /* Wait until the Transmit FIFO is NOT full (TXFF bit 5) */
    while (ioread32(uart_base + UART_FR) & TXFF) {
        cpu_relax(); /* Tell the CPU we are in a spin-loop to save power */
    }
    /* Write the character to the Data Register */
    iowrite32(c, uart_base + UART_DR);
}

/* Function to send a string character by character */
static void uart_send_string(char *str) {
    while (*str) 
    {
        uart_send_char(*str++);
    }
}

//IOCTL function to get the PID of the background user pgm running to send a signal back to it
static long basic_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{

    switch (cmd)
    {
        case IOCTL_SET_VAL:
            if (copy_from_user(&pid, ((int  __user*)arg), sizeof(int)))
                return -EFAULT;
            break;
   
        default:
                return -EINVAL;
    }

    return 0;
}

//the tasklet function used as bottom half
//tasklet runs in the interrupt context , so it should not sleep
void ir_sensor_tasklet_fn(unsigned long data) {
    //string to store the received sensor data
    char tx_buffer[64];

    pr_info("Inside Tasklet : Bottom Half\n");

    //formatting the string to send the data to PC via UART2
    int len = snprintf(tx_buffer, sizeof(tx_buffer), "PIR Detect #%d\r\n", motion_count);
    
    // Send string directly to registers
    uart_send_string(tx_buffer);
    
    pr_info("IR_DRIVER: Sent via Registers: %s", tx_buffer);

    //after an object is detected we are allowing the user to give permission only one time to access the camera and if they try to....
    //...give permission multiple times for one detection it should not work and it should print "object not detected" in the uart console
    permission_count=1;
}

//tasklet to define and trigger the userspace pgm 
void display_tasklet_fn(unsigned long data)
{
   struct pid *pid_struct;  //structure pointer to store the pid struct for the given pid of the userspace program to which we want to send the signal
   struct task_struct *task;  //task_struct pointer to store the task struct of the userspace program to which we want to send the signal

    pid_struct = find_get_pid(pid);   //find the pid struct for the given pid
    if (!pid_struct) {
        pr_err("Failed to find PID struct for PID %d\n", pid);
        return;
    }

    task = get_pid_task(pid_struct, PIDTYPE_PID);  //why PIDTYPE_PID => because we are looking for the task struct for the given pid and not for the given tgid or something else
    if (!task) {
        pr_err("Failed to find task for PID %d\n", pid);
        return; 
    }
    else{
        pr_info("Found task for PID %d, sending signal\n", pid);

        //whenever we get an object detection and if we try to give multiple times 0 and 1 it should not work....
        //for one object detection only 1 time the user should be able tp give the permission (one per detection)
        if(permission_count==1)
        {
            if(uart_interrupt_rx_data == '1')
                send_sig(SIGUSR1, task, 0); // Send SIGUSR1 to the userspace program
            else if(uart_interrupt_rx_data == '0')
                send_sig(SIGUSR2, task, 0); // Send SIGUSR2 to the userspace program
        }
        else  //if we are trying to give permission multiple times for one detection it should not work and it should print "object not detected" in the uart console
        {
            uart_send_string("object not detected permission denied\r\n ");
        }
        //increment the permission count to avoid giving multiple permissions for one detection
        permission_count++;

    }    

    put_task_struct(task); // Decrement the reference count of the task_struct
    put_pid(pid_struct);  // Decrement the reference count of the pid_struct
}
//basically tasklet cannot sleep and it runs in interrup context but in bottom half => defferred work


//Define the Interrupt Handler => (Top Half)
static irqreturn_t ir_sensor_irq_handler(int irq, void *dev_id) {
    motion_count++;

    pr_info("Interrupt Occured : In Top half\n");

    //wake up the tasklet
    tasklet_schedule(&ir_tasklet);

    return IRQ_HANDLED;
    //Top half done within very less amount of time
}

static irqreturn_t uart_isr(int irq, void *dev_id)
{
    unsigned int status;  //variable to store the interrupt status read from the UART_MIS register
    char data;   //character stores the data read from the UART_DR register when the interrupt is triggered and we are in the ISR context
    pr_info("in isr!\n");

    /*
       UART_MIS register
        Bit 4 (RXMIS): Receive interrupt. Data is waiting in the FIFO.
        Bit 5 (TXMIS): Transmit interrupt. There is space in the FIFO to send more data.
        Bit 6 (RTMIS): Receive Timeout interrupt. Data has been sitting in the FIFO for too long without enough new bytes arriving to hit the trigger level.
    */

    status = readl(uart_base + UART_MIS);

    // Check RX interrupt
    if (status & (1 << 4))   // RX interrupt
    {  
        if(*(int*)dev_id==1)
        {
             pr_info("inside isr\n");
             uart_interrupt_rx_data = readl(uart_base + UART_DR);
             //rxbuff[len] = readl(uart_base + UART_DR);
             pr_info("Received: %c\n",uart_interrupt_rx_data);
             //rxbuff[len]='\0';
        }

        // Clear interrupt
        writel((1 << 4), uart_base + UART_ICR);

        tasklet_schedule(&display_tasklet);
    }

    return IRQ_HANDLED;
}


//uart initializing registers function
static void uart2_init(void) {
    // 1. Disable UART while configuring (Clear all bits)
    iowrite32(0, uart_base + UART_CR);

    // 2. Set Baud Rate Divisors for 115200
    iowrite32(26, uart_base + UART_IBRD); 
    iowrite32(3, uart_base + UART_FBRD);

    /* 
     * 3. Line Control (UART_LCRH):
     * (1 << 5) | (1 << 6) : WLEN bits (Set to 11 for 8-bit word length)
     * Note: Bit 4 (FEN) is NOT set, so FIFO is DISABLED.
     * UART_LCRH bits:
            Bit 6:5 - WLEN (11 = 8 bits)
            Bit 4   - FEN  (0  = FIFO Disabled)
            Bit 3   - STP2 (0  = 1 Stop Bit)
            Bit 2   - EPS  (0  = Irrelevant if PEN is 0)
            Bit 1   - PEN  (0  = Parity Disabled)
     */
    iowrite32((1 << 5) | (1 << 6), uart_base + UART_LCRH); 

    // 4. Clear all previous interrupts (Write 1 to clear all 11 bits)
    iowrite32(0x7FF, uart_base + UART_ICR); 
    
    /* 
     * 5. Mask Interrupts (UART_IMSC):
     * (1 << 4) : RXIM (Receive Interrupt Mask)
     * (1 << 6) : RTIM (Receive Timeout Interrupt Mask)
     */
    iowrite32((1 << 4) | (1 << 6), uart_base + UART_IMSC);

    /* 
     * 6. Control Register (UART_CR):
     * (1 << 0) : UARTEN (Enable UART)
     * (1 << 8) : TXE    (Transmit Enable)
     * (1 << 9) : RXE    (Receive Enable)
     */
    iowrite32((1 << 0) | (1 << 8) | (1 << 9), uart_base + UART_CR);


    /*iowrite32 is a Linux kernel function used to write a 
    32-bit value to a specific hardware memory address (I/O memory).
    Syntax: iowrite32(value, address);
    iowrite32(data , reg address) has a memory barrier like : It forces the CPU to finish the write right now and in the .....
    .....correct order before moving to the next line of code.
    */

    
    pr_info("IR_DRIVER: UART2 initialized at 115200 baud (FIFO Disabled)\n");
}

/* Link the ioctl to the file operations */
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = basic_ioctl,
};


//Module Initialization
static int __init gpio_interrupt_init(void)
{


    //allocate the major number dynamically
    if(alloc_chrdev_region(&dev_num , 0 , 1 , DEVICE_NAME) < 0)
    {
        pr_err("Failed to allocate major number\n");
        return -1;
    }

    //create the device class (shows up in /sys/class)
    ir_class = class_create(DEVICE_NAME);
    if (IS_ERR(ir_class)) {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(ir_class);
    }


    //Creating cdev structure and adding the character  device to the system
    cdev_init(&ir_cdev , &fops);
    if(cdev_add(&ir_cdev , dev_num , 1) < 0)
    {        
        pr_err("Failed to add cdev\n");
        device_destroy(ir_class, dev_num);
        class_destroy(ir_class);
        unregister_chrdev_region(dev_num , 1);
        return -1;
    }


    if (IS_ERR(device_create(ir_class, NULL, dev_num, NULL, DEVICE_NAME))) {
        class_destroy(ir_class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    pr_info("IR_DRIVER: Device node /dev/%s created\n", DEVICE_NAME);
    
    
    //GPIO and UART initialization code starts here
    int ret;   //to collect the return value for irq_request function

    //Mapping the physical address to virtual space
    gpio_addr = ioremap(GPIO_BASE_PHYS, GPIO_SIZE);

    if(!gpio_addr)
    {
        pr_err("Failed to map GPIO memory!!\n");
        return -ENOMEM;
    }

    //request the GPIO PIN 17
    gpio_request(GPIO_IR_SENSOR, "IR_PIN");

    //set the direction of the GPIO PIN as input ...tells that the specififed GPIO PIN 17 should be input
    gpio_direction_input(GPIO_IR_SENSOR);


    // 1. Map UART Registers into Virtual Memory
    uart_base = ioremap(UART_BASE_PHYS, UART_SIZE);
    if (!uart_base) {
        pr_err("IR_DRIVER: Failed to ioremap UART2 registers\n");
        return -ENOMEM;
    }

    //function to initialize the UART hardware 
    uart2_init();
    
    //sending a string to know that uart tx has started
    uart_send_string("transmission started\n");

    if (request_irq(UART_IRQ, uart_isr, IRQF_SHARED, "uart_irq1",&k))
    {
        printk("IRQ request failed\n");
        return -1;
    }

    //setup the tasklet or nitialize the tasklet function
    tasklet_init(&ir_tasklet, ir_sensor_tasklet_fn, 0);
    tasklet_init(&display_tasklet, display_tasklet_fn, 0);

    //map the interrupt to the gpio
    ir_irq_number = gpio_to_irq(GPIO_IR_SENSOR);
    if(ir_irq_number < 0)
    {
        printk(KERN_INFO "Failed to get IRQ for GPIO\n");
        gpio_free(GPIO_IR_SENSOR);
        return 0;
    }
   
    pr_info("IRQ number -> %d\n" ,ir_irq_number); 
    

    //this irq request is for requesting irq for gpio(GPIO interrupt)
    ret = request_irq(ir_irq_number, ir_sensor_irq_handler, IRQF_TRIGGER_FALLING , DEVICE_NAME , NULL);
    //flag -> IRQF_TRIGGER_RISING => telling that the interrupt should occur for rising 0 to 1 transition
    //request for the irq
    if(ret)
    {
        pr_err("IR_DRIVER: Failed to request IRQ\n");
        iounmap(gpio_addr);
        iounmap(uart_base);
        gpio_free(GPIO_IR_SENSOR);
        return ret;
    }


    pr_info("Module initializing completed\n");

    return 0;

}

//Unloading module (Exit function)
static void __exit gpio_interrupt_exit(void)
{
    // 1. Send goodbye message
    uart_send_string("Driver Unloading...\r\n");

    // 2. Free Interrupts
    free_irq(ir_irq_number, NULL);
    free_irq(UART_IRQ, &k);

    // 3. Stop Tasklets
    tasklet_kill(&ir_tasklet);
    tasklet_kill(&display_tasklet);

    // 4. Destroy /dev/ node and class
    device_destroy(ir_class, dev_num);
    cdev_del(&ir_cdev);
    class_destroy(ir_class);
    unregister_chrdev_region(dev_num, 1);

    // 5. Unmap and release GPIO
    iounmap(gpio_addr);
    iounmap(uart_base);
    gpio_free(GPIO_IR_SENSOR);

    pr_info("IR_DRIVER: Module unloaded successfully\n");
}


module_init(gpio_interrupt_init);
module_exit(gpio_interrupt_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SIDDARTH M");
MODULE_DESCRIPTION("This is a basic GPIO interrupt interfaced with a IR sensor");
