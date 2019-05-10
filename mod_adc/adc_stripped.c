#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/mutex.h>
#include <mach/hardware.h>
#include <mach/platform.h>
#include <mach/irqs.h>
#include <asm/uaccess.h>

#define DEVICE_NAME 		"adc"
#define ADC_NUMCHANNELS		3

// adc registers
#define	ADCLK_CTRL			io_p2v(0x400040B4)
#define	ADCLK_CTRL1			io_p2v(0x40004060)
#define	ADC_SELECT			io_p2v(0x40048004)
#define	ADC_CTRL			io_p2v(0x40048008)
#define ADC_VALUE           io_p2v(0x40048048)
#define SIC2_ATR            io_p2v(0x40010010)

#define SIC1_ER             io_p2v(0x4000C000)
#define ADC_VAL_MASK 0x3FF

#define ADC_CTRL_MASK       (1<<2)

#define READ_REG(a)         (*(volatile unsigned int *)(a))
#define WRITE_REG(b,a)      (*(volatile unsigned int *)(a) = (b))

DECLARE_WAIT_QUEUE_HEAD(adc_wait_event);
static DEFINE_MUTEX(adc_mutex);
static bool conversion_is_done = false;

static unsigned char    adc_channel = 0;
static int              adc_values[ADC_NUMCHANNELS] = {0, 0, 0};


static irqreturn_t      adc_interrupt(int irq, void * dev_id);
static irqreturn_t      gp_interrupt(int irq, void * dev_id);


static void adc_init(void)
{
	unsigned long data;

	// set 32 KHz RTC clock
    data = READ_REG(ADCLK_CTRL);
    data |= 0x1;
    WRITE_REG(data, ADCLK_CTRL);

	// rtc clock ADC & Display = from PERIPH_CLK
    data = READ_REG(ADCLK_CTRL1);
    data &= ~0x01ff;
    WRITE_REG(data, ADCLK_CTRL1);

	// negative & positive reference
    data = READ_REG(ADC_SELECT);
    data &= ~0x03c0;
    data |=  0x0280;
    WRITE_REG(data, ADC_SELECT);

	// switch on adc and reset
    data = READ_REG(ADC_CTRL);
    data |= ADC_CTRL_MASK;
    WRITE_REG(data, ADC_CTRL);

    // enable touch screen interrupt
    data = READ_REG(SIC1_ER);
    data |= IRQ_LPC32XX_TS_IRQ;
    WRITE_REG(data, SIC1_ER);

	//IRQ init
    if (request_irq(IRQ_LPC32XX_TS_IRQ, adc_interrupt, IRQF_DISABLED, "adc_interrupt", NULL) != 0)
    {
        printk(KERN_ALERT "ADC IRQ request failed\n");
    }

    // Set gp_interrupt trigger to rising edge
    data = READ_REG(SIC2_ATR);
    data |= _BIT(23);
    WRITE_REG(data, SIC2_ATR);

    if (request_irq(IRQ_LPC32XX_GPI_01, gp_interrupt, IRQF_DISABLED, "gpi_interrupt", NULL) != 0)
    {
        printk(KERN_ALERT "GP IRQ request failed\n");
    }
}

static void adc_start(unsigned char channel)
{
    if(mutex_lock_interruptible(&adc_mutex))
    {
        printk(KERN_WARNING "ADC Conversion already in progress");
        return;
    }

	unsigned long data;

	if (channel >= ADC_NUMCHANNELS)
    {
        channel = 0;
    }

	data = READ_REG(ADC_SELECT);
    // select the channel, first read it, ignore channel bits and change only the channel bits (0x0030)
	WRITE_REG((data & ~0x0030) | ((channel << 4) & 0x0030), ADC_SELECT);

    // remember the channel globally so we can recognise it later on
	adc_channel = channel;

	// start conversion
    data = READ_REG(ADC_CTRL);
    data |= _BIT(1);
    WRITE_REG(data, ADC_CTRL);


}

static irqreturn_t adc_interrupt(int irq, void * dev_id)
{
    adc_values[adc_channel] = READ_REG(ADC_VALUE) & ADC_VAL_MASK0x3FF;
    printk(KERN_WARNING "ADC(%d)=%d\n", adc_channel, adc_values[adc_channel]);

    // start the next channel:
    adc_channel++;
    mutex_unlock(&adc_mutex);
    if (adc_channel < ADC_NUMCHANNELS)
    {
        adc_start(adc_channel);
    }

    conversion_is_done = true;
    wake_up_interruptible(&adc_wait_event);
    return (IRQ_HANDLED);
}

static irqreturn_t gp_interrupt(int irq, void * dev_id)
{
    conversion_is_done = false;
    adc_start(0);

    return (IRQ_HANDLED);
}

static void adc_exit(void)
{
    free_irq(IRQ_LPC32XX_TS_IRQ, NULL);
    free_irq(IRQ_LPC32XX_GPIO_01, NULL);
}

static ssize_t device_read (struct file * file, char __user * buf, size_t length, loff_t * f_pos)
{
	int     channel = (int) file->private_data;
    int     bytes_read = 0;
    int     bytes_written = 0;
    char    outMsg[128];

    printk(KERN_WARNING DEVICE_NAME ":device_read(%d)\n", channel);

    if(channel < 0 || channel >= ADC_NUMCHANNELS)
    {
		return -EFAULT;
    }

    conversion_is_done = false;
    adc_start(channel);
    wait_event_interruptible(adc_wait_event, conversion_is_done);

    bytes_written = sprintf(outMsg, "%d", adc_values[channel]);
    bytes_read = copy_to_user(buf, outMsg, bytes_written + 1);

    return bytes_read;
}

static int device_open (struct inode * inode, struct file * file)
{
    // get channel from 'inode'
    int channel = MINOR(inode->i_rdev);;
    file->private_data = (void*)channel;

    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release (struct inode * inode, struct file * file)
{
    printk (KERN_WARNING DEVICE_NAME ": device_release()\n");

    module_put(THIS_MODULE);
	return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .read = device_read,
    .open = device_open,
    .release = device_release
};

static struct chardev
{
    dev_t       dev;
    struct cdev cdev;
} adcdev;

int adcdev_init(void)
{
    // try to get a dynamically allocated major number
	int error = alloc_chrdev_region(&adcdev.dev, 0, ADC_NUMCHANNELS, DEVICE_NAME);;

	if(error < 0)
	{
		// failed to get major number for our device.
		printk(KERN_WARNING DEVICE_NAME ": dynamic allocation of major number failed, error=%d\n", error);
		return error;
	}

	printk(KERN_INFO DEVICE_NAME ": major number=%d\n", MAJOR(adcdev.dev));

	cdev_init(&adcdev.cdev, &fops);
	adcdev.cdev.owner = THIS_MODULE;
	adcdev.cdev.ops = &fops;

	error = cdev_add(&adcdev.cdev, adcdev.dev, ADC_NUMCHANNELS);
	if(error < 0)
	{
		// failed to add our character device to the system
		printk(KERN_WARNING DEVICE_NAME ": unable to add device, error=%d\n", error);
		return error;
	}


	adc_init();


	return 0;
}


/*
 * Cleanup - unregister the appropriate file from /dev
 */
void cleanup_module()
{
	cdev_del(&adcdev.cdev);
	unregister_chrdev_region(adcdev.dev, ADC_NUMCHANNELS);

	adc_exit();
}


module_init(adcdev_init);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Joris Geurts, Tobias Neyssen & Leon Leijssen");
MODULE_DESCRIPTION("ADC Driver");

