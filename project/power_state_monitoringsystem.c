/*
 * LINUX KERNEL BASED POWER STATE MONITORING SYSTEM USING RTC
 *
 * Linux kernel driver for DS3231 RTC with UART status output,
 * PWM LED brightness, CPU governor control, and power states
 * (RUNNING / IDLE / LOW_POWER) based on keyboard and UART activity.
 *
 * Author: Anjali&Pavani
 */

/* Standard kernel headers for module, I2C, timers, threads, etc. */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/io.h>
#include <linux/delay.h>
#include <linux/notifier.h>
#include <linux/jiffies.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/keyboard.h>
#include <linux/pwm.h>
#include <linux/mutex.h>

/*
 * Mutex lock for UART transmit.
 * Only one part of the driver should write to UART at a time
 * so messages do not get mixed on the serial line.
 */
static DEFINE_MUTEX(uart_lock);

/*
 * Wait queue for the RTC kernel thread.
 * The timer wakes this queue every 1 second so the thread can run.
 */
static DECLARE_WAIT_QUEUE_HEAD(rtc_wq);

/*
 * Flag set by timer interrupt handler.
 * When 1, the RTC thread knows it should do its 1-second work.
 */
static int timer_event = 0;

/* ------------------------------------------------ */
/* DS3231 Registers */
/* Register addresses inside the DS3231 RTC chip    */
/* ------------------------------------------------ */

#define DS3231_SEC     0x00   /* seconds register */
#define DS3231_MIN     0x01   /* minutes register */
#define DS3231_HOUR    0x02   /* hours register */
#define DS3231_DAY     0x03   /* day-of-week register (1=Sunday, etc.) */
#define DS3231_DATE    0x04   /* date of month register */
#define DS3231_MON     0x05   /* month register */
#define DS3231_YEAR    0x06   /* year register (00-99) */

/* ------------------------------------------------ */
/* PWM */
/* PWM period for LED brightness control (1 ms)     */
/* ------------------------------------------------ */

#define PWM_PERIOD_NS   1000000

/* ------------------------------------------------ */
/* UART0 Physical Base */
/* Raspberry Pi PL011 UART0 mapped address          */
/* ------------------------------------------------ */

#define UART0_BASE_PHYS   0xFE201000
#define UART0_SIZE        0x90

/* UART register offsets from uart_base */

#define UART_DR     0x00   /* data register - send/receive one byte */
#define UART_FR     0x18   /* flag register - FIFO full/empty status */
#define UART_IBRD   0x24   /* integer part of baud rate divisor */
#define UART_FBRD   0x28   /* fractional part of baud rate divisor */
#define UART_LCRH   0x2C   /* line control (8-bit, FIFO enable) */
#define UART_CR     0x30   /* control register - enable UART TX/RX */
#define UART_ICR    0x44   /* interrupt clear register */

#define FR_TXFF     (1 << 5)   /* transmit FIFO full - wait before writing */
#define FR_RXFE     (1 << 4)   /* receive FIFO empty - no data to read */

/* ------------------------------------------------ */
/* DAYS */
/* Day names for printing (DS3231 day register 1-7) */
/* ------------------------------------------------ */

static const char *days[] = {

	"SUN",
    	"MON",
    	"TUE",
    	"WED",
    	"THU",
    	"FRI",
    	"SAT"
};

/* ------------------------------------------------ */
/* GLOBALS */
/* Driver-wide variables used across functions      */
/* ------------------------------------------------ */

/* I2C client pointer - set in probe, used to read RTC */
static struct i2c_client *g_client;

/* Kernel timer - fires every 1 second */
static struct timer_list kernel_timer;

/* Main worker thread - reads RTC and updates power state */
static struct task_struct *rtc_thread;

/* Virtual address after ioremap of UART hardware registers */
static void __iomem *uart_base;

/*
 * Last time (in jiffies) when user activity was seen
 * (keyboard or UART). Used to decide IDLE / LOW_POWER.
 */
static unsigned long last_activity;

/* Current power mode name stored as string for printing */
static char current_state[32] = "RUNNING";

/* Keyboard notifier block - registered with kernel keyboard layer */
static struct notifier_block kb_nb;

/* PWM device for LED brightness on the board */
static struct pwm_device *led_pwm;

/* ------------------------------------------------ */
/* KEYBOARD EVENT THREAD */
/* Separate thread wakes on key press (after debounce) */
/* ------------------------------------------------ */

/* Wait queue for keyboard thread */
static DECLARE_WAIT_QUEUE_HEAD(kb_wq);

/* Keyboard handling thread */
static struct task_struct *kb_thread;

/* Set to 1 when a valid key press is detected */
static int kb_event = 0;

/* Time of last accepted key press - used for debouncing */
static unsigned long last_key_jiffies = 0;

/* Ignore key repeats within 200 ms */
#define KEY_DEBOUNCE_MS   200

/* ------------------------------------------------ */
/* PWM BRIGHTNESS */
/* Change LED duty cycle: 0% = off, 100% = full on  */
/* ------------------------------------------------ */

static void set_led_brightness(int percent)
{
   int duty;

    /* If PWM was not obtained in probe, do nothing */
   if (!led_pwm)
        return;

    /* Keep percent in valid range 0 to 100 */
    if (percent < 0)
        percent = 0;

    if (percent > 100)
        percent = 100;

    /* Calculate on-time within one PWM period */
    duty = (PWM_PERIOD_NS * percent) / 100;

    /* Apply duty cycle and period to hardware PWM */
    pwm_config(led_pwm,duty,PWM_PERIOD_NS);
    if (percent == 0)
        pwm_disable(led_pwm);
    else
        pwm_enable(led_pwm);
}

/* ------------------------------------------------ */
/* UART FUNCTIONS */
/* Direct register access to PL011 UART for output  */
/* ------------------------------------------------ */

/*
 * uart_init_hw()
 * Configure UART for 115200 baud, 8 data bits, FIFO enabled.
 * Called once during probe after ioremap.
 */
static void uart_init_hw(void)
{
    /* Disable UART before changing settings */
    writel(0x0,uart_base + UART_CR);

    /* Clear all pending UART interrupts */
    writel(0x7FF,uart_base + UART_ICR);

    /* Baud rate divisors for 115200 @ 48 MHz UART clock */
    writel(26,uart_base + UART_IBRD);

    writel(3,uart_base + UART_FBRD);

    /* 8-bit words, FIFO enabled */
    writel((1 << 4) |(1 << 5) |(1 << 6),uart_base + UART_LCRH);

    /* Enable UART, transmitter, and receiver */
    writel((1 << 0) | (1 << 8) |(1 << 9),uart_base + UART_CR);

    pr_info("UART Initialized\n");
}

/*
 * uart_putc()
 * Send one character. Waits if transmit FIFO is full.
 */
static void uart_putc(char c)
{
    while (readl(uart_base + UART_FR) & FR_TXFF)
        cpu_relax();

    writel(c,uart_base + UART_DR);
}

/*
 * uart_send_string()
 * Send a full string over UART. Uses mutex so prints do not overlap.
 * Adds carriage return after newline for terminal compatibility.
 */
static void uart_send_string(const char *str)
{
    mutex_lock(&uart_lock);
    while (*str)
    {

        uart_putc(*str);

        if (*str == '\n')
            	uart_putc('\r');

        str++;
    }
    mutex_unlock(&uart_lock);
}

/* ------------------------------------------------ */
/* CPU GOVERNOR */
/* Write scaling governor name to sysfs (performance/ondemand) */
/* ------------------------------------------------ */

/*
 * set_cpu_governor()
 * Changes CPU frequency policy by writing to cpufreq sysfs.
 * "performance" = max speed, "ondemand" = save power when idle.
 */
static void set_cpu_governor(const char *gov)
{
    struct file *f;

    loff_t pos = 0;

    char buf[32];

    snprintf(buf,sizeof(buf),"%s\n",gov);

    f = filp_open("/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor",O_WRONLY,0);

    if (IS_ERR(f)) 
    {

        pr_err("Governor file open failed\n");

        return;
    }

    ssize_t ret;

    ret=kernel_write(f,buf,strlen(buf),&pos);

    if(ret<0)
	pr_err("Governor write failed\n");
	
    filp_close(f,NULL);

    pr_info("Governor -> %s\n",gov);
}

/* ------------------------------------------------ */
/* POWER PRINT */
/* Send current power mode name to UART and kernel log */
/* ------------------------------------------------ */

static void print_power_state(const char *state)
{
    char msg[64];

    snprintf(msg,sizeof(msg),"\nPOWER MODE: %s\n",state);

    uart_send_string(msg);

    pr_info("%s", msg);
}

/* ------------------------------------------------ */
/* ACTIVITY UPDATE */
/* Called when keyboard or UART shows user activity   */
/* ------------------------------------------------ */

/*
 * update_activity()
 * Refreshes idle timer. If we were in IDLE or LOW_POWER,
 * wake up to RUNNING (performance governor, LED 100%).
 */
static void update_activity(void)
{
    last_activity = jiffies;

    if (strcmp(current_state,"IDLE") == 0 ||strcmp(current_state,"LOW_POWER") == 0) 
    {

        strcpy(current_state,"WAKEUP");

        set_cpu_governor("performance");

        set_led_brightness(100);

        print_power_state("WAKEUP");

        strcpy(current_state,"RUNNING");

        print_power_state("RUNNING");
    }
}

/* ------------------------------------------------ */
/* UART ACTIVITY */
/* If user sent data on UART RX, treat as activity    */
/* ------------------------------------------------ */

static void check_uart_activity(void)
{
    u32 fr;

    /* Read UART flag register */
    fr = readl(uart_base + UART_FR);

    /* FR_RXFE clear means receive FIFO has at least one byte */
    if (!(fr & FR_RXFE)) 
    {

        /* Read and discard byte (clear RX); marks activity */
        readl(uart_base + UART_DR);

        update_activity();
    }
}

/* ------------------------------------------------ */
/* KEYBOARD NOTIFIER */
/* Kernel calls this on key events; we debounce     */
/* and wake keyboard thread                         */
/* ------------------------------------------------ */

static int keyboard_event(struct notifier_block *nblock,unsigned long code,void *param)
{
    struct keyboard_notifier_param *kp = param;

    unsigned long now;

    /* Only care about actual key symbol events */
    if (code != KBD_KEYSYM)
        return NOTIFY_OK;

    /* Ignore key release; only key press */
    if (!kp->down)
        return NOTIFY_OK;

    now = jiffies;

    /* ------------------------------------------------ */
    /* DEBOUNCE LOGIC */
    /* Ignore keys that come too fast (same key bounce) */
    /* ------------------------------------------------ */

    if (time_before(now,last_key_jiffies + msecs_to_jiffies(KEY_DEBOUNCE_MS))) 
    {

        return NOTIFY_OK;
    }

    last_key_jiffies = now;

    /* Tell keyboard thread to run update_activity() */
    kb_event = 1;

    wake_up_interruptible(&kb_wq);

    return NOTIFY_OK;
}

/* ------------------------------------------------ */
/* RTC FUNCTIONS */
/* ------------------------------------------------ */

/* Convert BCD byte from RTC chip to normal decimal number */
static int bcd_to_dec(u8 val)
{
    return ((val >> 4) * 10) + (val & 0x0F);
}

/* Read one byte register from DS3231 over I2C */
static int ds3231_read_reg(struct i2c_client *client,u8 reg)
{
    	return i2c_smbus_read_byte_data(client,reg);
}

/* ------------------------------------------------ */
/* RTC READ */
/* Read all time registers and print on UART        */
/* ------------------------------------------------ */

static void ds3231_read_time(struct i2c_client *client)
{
    int sec;
    int min;
    int hour;

    int day;
    int date;
    int mon;
    int year;

    char msg[128];

    /* Read raw register bytes from DS3231 */
    sec  = ds3231_read_reg(client,DS3231_SEC);

    min  = ds3231_read_reg(client,DS3231_MIN);

    hour = ds3231_read_reg(client,DS3231_HOUR);

    day  = ds3231_read_reg(client,DS3231_DAY);

    date = ds3231_read_reg(client,DS3231_DATE);

    mon  = ds3231_read_reg(client,DS3231_MON);

    year = ds3231_read_reg(client,DS3231_YEAR);

    /* Clear CH (clock halt) bit in seconds if present */
    sec &= 0x7F;

    /* Convert BCD values to decimal for display */
    sec  = bcd_to_dec(sec);

    min  = bcd_to_dec(min);

    hour = bcd_to_dec(hour & 0x3F);

    day  = bcd_to_dec(day);

    date = bcd_to_dec(date);

    mon  = bcd_to_dec(mon & 0x1F);

    year = bcd_to_dec(year);

    /* Format: time, date, day name */
    snprintf(msg, sizeof(msg),"RTC %02d:%02d:%02d %02d/%02d/%02d %s\n",hour,min,sec,date,mon,year,days[day - 1]);

    uart_send_string(msg);

    pr_info("%s", msg);
}

/* ------------------------------------------------ */
/* TIMER FUNCTION */
/* Runs in timer context every 1 second             */
/* ------------------------------------------------ */

/*
 * timer_fn()
 * Lightweight: only set flag and wake RTC thread.
 * I2C and UART work is done in thread_fn(), not here.
 */
static void timer_fn(struct timer_list *t)
{
    timer_event = 1;
    wake_up(&rtc_wq);

    /* Schedule next tick in 1 second */
    mod_timer(&kernel_timer,jiffies + msecs_to_jiffies(1000));
}

/* ------------------------------------------------ */
/* KEYBOARD THREAD */
/* Handles activity update when key was pressed     */
/* ------------------------------------------------ */

static int keyboard_thread_fn(void *data)
{
    while (!kthread_should_stop()) 
    {
	/* Sleep until keyboard_event sets kb_event */
	wait_event_interruptible(kb_wq,kb_event ||kthread_should_stop());

        if (kthread_should_stop())
            	break;

        kb_event = 0;

        /* Key press detected - refresh activity / wake from idle */
        update_activity();
    }

    return 0;
}

/* ------------------------------------------------ */
/* RTC THREAD */
/* Main loop: RTC read + idle power state machine   */
/* ------------------------------------------------ */

/*
 * thread_fn()
 * Woken every second by timer. Then:
 * 1) Read and print RTC time
 * 2) Check UART for incoming data (activity)
 * 3) If no activity for >10s -> LOW_POWER
 *    else if >5s -> IDLE
 *    else -> RUNNING
 */
static int thread_fn(void *data)
{
    while (!kthread_should_stop()) 
    {

        wait_event_interruptible(rtc_wq,timer_event || kthread_should_stop());

        if (kthread_should_stop())
            	break;

        timer_event = 0;

        /* 1. READ RTC EVERY SECOND */
        if (g_client)
            	ds3231_read_time(g_client);

        /* 2. CHECK ACTIVITY */
        check_uart_activity();

        /* 3. UPDATE POWER STATE LOGIC */
        unsigned long diff = jiffies - last_activity;

        /* No activity for more than 10 seconds */
        if (time_after(diff, msecs_to_jiffies(10000))) 
	{

            if (strcmp(current_state, "LOW_POWER") != 0) 
	    {

                strcpy(current_state, "LOW_POWER");
                set_cpu_governor("ondemand");
                set_led_brightness(10);

                print_power_state("LOW_POWER");
            	}

        }
	/* No activity for more than 5 seconds (but less than 10) */
	else if (time_after(diff, msecs_to_jiffies(5000))) 
	{

            if (strcmp(current_state, "IDLE") != 0) 
	    {

                strcpy(current_state, "IDLE");
                set_led_brightness(40);

                print_power_state("IDLE");
            }

        } 
	/* Recent activity - normal running mode */
	else 
	{

            if (strcmp(current_state, "RUNNING") != 0) 
	    {

                strcpy(current_state, "RUNNING");
                set_cpu_governor("performance");
                set_led_brightness(100);

                print_power_state("RUNNING");
            }
        }
    }

    	return 0;
}
/* ------------------------------------------------ */
/* PROBE */
/* Called when kernel binds driver to DS3231 device */
/* ------------------------------------------------ */

static int ds3231_probe(struct i2c_client *client)
{
    g_client = client;

    last_activity = jiffies;

    /* Map UART physical address into kernel virtual memory */
    uart_base = ioremap(UART0_BASE_PHYS, UART0_SIZE);

    if (!uart_base) 
    {

        pr_err("UART ioremap failed\n");

        return -ENOMEM;
    }

    uart_init_hw();

    /* Get PWM channel for LED from device tree */
    led_pwm = pwm_get(&client->dev,NULL);

    if (IS_ERR(led_pwm)) 
    {

        pr_err("PWM get failed\n");

        iounmap(uart_base);

        return PTR_ERR(led_pwm);
    }

    pwm_config(led_pwm,PWM_PERIOD_NS,PWM_PERIOD_NS);

    pwm_enable(led_pwm);

    strcpy(current_state,"BOOTING");

    set_led_brightness(100);

    print_power_state("BOOTING");

    set_cpu_governor("performance");

    /* Register callback for keyboard events */
    kb_nb.notifier_call = keyboard_event;

    register_keyboard_notifier(&kb_nb);

    /* Start thread that runs update_activity on key press */
    kb_thread = kthread_run(keyboard_thread_fn,NULL,"kb_thread");

    if (IS_ERR(kb_thread)) 
    {

    	unregister_keyboard_notifier(&kb_nb);

    	pwm_put(led_pwm);

    	iounmap(uart_base);

    	return PTR_ERR(kb_thread);
    }
    /* Start main RTC + power management thread */
    rtc_thread = kthread_run(thread_fn,NULL,"rtc_thread");

    if (IS_ERR(rtc_thread)) 
    {
	unregister_keyboard_notifier(&kb_nb);
	pwm_put(led_pwm);

        iounmap(uart_base);

        return PTR_ERR(rtc_thread);
    }
		
    /* Start 1 Hz periodic timer */
    timer_setup(&kernel_timer,timer_fn,0);

    mod_timer(&kernel_timer,jiffies + msecs_to_jiffies(1000));

    strcpy(current_state,"RUNNING");

    set_led_brightness(100);

    print_power_state("RUNNING");

    pr_info("Driver Probe Success\n");

    return 0;
}

/* ------------------------------------------------ */
/* REMOVE */
/* Called when module is unloaded or device removed */
/* ------------------------------------------------ */

static void ds3231_remove(struct i2c_client *client)
{
    strcpy(current_state,"SHUTDOWN");

    print_power_state("SHUTDOWN");

    set_cpu_governor("performance");
    del_timer_sync(&kernel_timer);

    if (rtc_thread)
        kthread_stop(rtc_thread);

    if (kb_thread)
    	kthread_stop(kb_thread);
    unregister_keyboard_notifier(&kb_nb);

    pwm_disable(led_pwm);

    pwm_put(led_pwm);

    /* Disable UART and release mapped memory */
    writel(0x0,uart_base + UART_CR);

    iounmap(uart_base);

    pr_info("Driver Removed\n");
}

/* ------------------------------------------------ */ 
/* DEVICE TREE */
/* Must match compatible string in your .dts file  */
/* ------------------------------------------------ */

static const struct of_device_id ds3231_dt_ids[] = {

    {
        .compatible = "maxim,ds3231-custom",
    },

    { }
};

MODULE_DEVICE_TABLE(of,ds3231_dt_ids);

/* ------------------------------------------------ */
/* I2C DRIVER */
/* Registers this driver with Linux I2C subsystem    */
/* ------------------------------------------------ */

static struct i2c_driver ds3231_driver = {

    .driver = {

        .name = "power_state_monitoringsystem",

        .of_match_table = ds3231_dt_ids,
    },

    .probe  = ds3231_probe,

    .remove = ds3231_remove,
};

/* Macro: module init registers i2c_driver, exit unregisters */
module_i2c_driver(ds3231_driver);

/* ------------------------------------------------ */
/* MODULE INFO */
/* ------------------------------------------------ */

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Anjali-Pavani");

MODULE_DESCRIPTION("Linux Kernel based power state monitoring system using rtc");

