#include <asm/uaccess.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <mach/hardware.h>

#define LCD_CFG         0x40004054
#define P0_DIR_SET      0x40028050
#define P0_DIR_CLR      0x40028054
#define P0_DIR_STATE    0x40028058
#define P0_INP_STATE    0x40028040
#define P0_OUTP_SET     0x40028044
#define P0_OUTP_CLR     0x40028048
#define P0_OUTP_STATE   0x4002804C

#define P1_MUX_SET      0x40028130
#define P1_MUX_CLR      0x40028134
#define P1_MUX_STATE    0x40028138
#define P1_DIR_SET      0x40028070
#define P1_DIR_CLR      0x40028074
#define P1_DIR_STATE    0x40028078
#define P1_INP_STATE    0x40028060
#define P1_OUTP_SET     0x40028064
#define P1_OUTP_CLR     0x40028068
#define P1_OUTP_STATE   0x4002806C
#define P1_MUX_SET_VALUE 0x00FFFFFF

#define P2_MUX_SET      0x40028028
#define P2_DIR_SET      0x40028010
#define P2_MUX_STATE    0x40028030
#define P2_DIR_CLR      0x40028014
#define P2_DIR_STATE    0x40028018
#define P2_INP_STATE    0x4002801C
#define P2_OUTP_SET     0x40028020
#define P2_OUTP_CLR     0x40028024
#define P2_MUX_SET_VALUE (1 << 3)

#define P3_INP_STATE    0x40028000
#define P3_OUTP_SET     0x40028004
#define P3_OUTP_CLR     0x40028008
#define P3_OUTP_STATE   0x4002800C

typedef struct pin_t{
    uint32_t PinNo;
    uint32_t Connector;
    uint32_t BitOffset;
    uint32_t ReadBitOffset;
    uint32_t RegSetOutput;
    uint32_t RegSetInput;
    uint32_t RegRead;
    uint32_t RegHigh;
    uint32_t RegLow;
} PIN;

static PIN pins[] = {
    // p0
    {40, 3, 0, 0, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {24, 2, 1, 1, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {11, 2, 2, 2, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {12, 2, 3, 3, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {13, 2, 4, 4, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {14, 2, 5, 5, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {33, 3, 6, 6, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    {27, 1, 7, 7, P0_DIR_SET, P0_DIR_CLR, P0_INP_STATE, P0_OUTP_SET, P0_OUTP_CLR},
    // p2
    {47, 3, 0, 0, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {56, 3, 1, 1, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {48, 3, 2, 2, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {57, 3, 3, 3, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {49, 3, 4, 4, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {58, 3, 5, 5, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {50, 3, 6, 6, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {45, 3, 7, 7, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {49, 1, 8, 8, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {50, 1, 9, 9, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {51, 1, 10, 10, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {52, 1, 11, 11, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    {53, 1, 12, 12, P2_DIR_SET, P2_DIR_CLR, P2_INP_STATE, P2_OUTP_SET, P2_OUTP_CLR},
    // p3
    {54, 3, 25, 10, P2_DIR_SET, P2_DIR_CLR, P3_INP_STATE, P3_OUTP_SET, P3_OUTP_CLR},
    {46, 3, 26, 11, P2_DIR_SET, P2_DIR_CLR, P3_INP_STATE, P3_OUTP_SET, P3_OUTP_CLR},
    {36, 3, 29, 14, P2_DIR_SET, P2_DIR_CLR, P3_INP_STATE, P3_OUTP_SET, P3_OUTP_CLR},
    {24, 1, 30, 25, P2_DIR_SET, P2_DIR_CLR, P3_INP_STATE, P3_OUTP_SET, P3_OUTP_CLR},

};

static PIN* getPin(uint32_t pin, uint32_t connector)
{
    int i;
    for(i=0; i<sizeof(pins)/sizeof(PIN); i++)
    {
        if(pins[i].PinNo == pin && pins[i].Connector == connector)
        {
            return &pins[i];
        }
    }
    return NULL;
}

static void SetPinOutput(PIN* pin)
{
    *((uint32_t*)io_p2v(pin->RegSetOutput)) |= (1<<pin->BitOffset);
}

static void SetPinInput(PIN* pin)
{
    *((uint32_t*)io_p2v(pin->RegSetInput)) |= (1<<pin->BitOffset);
}

static void SetPinHigh(PIN* pin)
{
    *((uint32_t*)io_p2v(pin->RegHigh)) |= (1<<pin->BitOffset);
}

static void SetPinLow(PIN* pin)
{
    *((uint32_t*)io_p2v(pin->RegLow)) |= (1<<pin->BitOffset);
}

static uint32_t ReadPin(PIN* pin)
{
    uint32_t curValue = *((uint32_t*)io_p2v(pin->RegRead));
    if ((curValue & (1<<pin->ReadBitOffset)) > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


#define sysfs_dir "es6"
#define sysfs_file "gpio"
#define device_name "gpio"
#define register_size 4
#define max_buffer_size 1024
#define SUCCESS 0

/* string for show */
static char result_string[max_buffer_size] = "";
static char dev_res_string[max_buffer_size] = "";
static PIN* curPin = NULL;

/* result of sscanf calls */
static int result = 0;

/* input parameters */
static char operation = 0;
static uint32_t connector = 0;
static uint32_t pin_number = 0;
static char pin_direction = 0;

static int major_number = 0;

/* device file can only be opened once */
static bool device_opened = 0;


static void initializePorts(void)
{
    *((uint32_t*)io_p2v(LCD_CFG)) = 0; // turn off LCD
    *((uint32_t*)io_p2v(P2_MUX_SET)) |= P2_MUX_SET_VALUE;
}

static ssize_t
sysfs_show(struct device *dev, struct device_attribute *attr, char *buffer)
{
    return sprintf(buffer, result_string);
}

static ssize_t
sysfs_store(struct device* dev, struct device_attribute* attr, const char* buffer, size_t count)
{
    PIN* pin = NULL;
    result = sscanf(buffer, "%c j%d.%d", &pin_direction, &connector, &pin_number);

    if(result != 3)
    {
        printk(KERN_WARNING "Expected 2 parameters got %d\n", result);
        return -EINVAL;
    }

    pin = getPin(pin_number, connector);
    if(pin == NULL)
    {
        printk(KERN_WARNING "Pin j%d.%d does not exist\n", pin_number, connector);
        return -EINVAL;
    }

    switch(pin_direction)
    {
        case 'i':
            SetPinInput(pin);
            break;
        case 'o':
            SetPinOutput(pin);
            break;
        default:
            printk(KERN_WARNING "Invalid operation: %c expected i or o\n", pin_direction);
            break;
    }

    printk(KERN_INFO "Connector<%d> pin<%d> value<%c>\n",
        connector, pin_number, pin_direction);
    sprintf(result_string,"Connector<%d> pin<%d> value<%c>\n",
        connector, pin_number, pin_direction);
    return count;
}

static DEVICE_ATTR(gpio, S_IWUGO | S_IRUGO , sysfs_show, sysfs_store);

static struct attribute* attrs[] = {
    &dev_attr_gpio.attr,
    NULL
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject* gpio_obj = NULL;


static int device_open(struct inode* inode, struct file* file)
{
    if(device_opened)
    {
        return -EBUSY;
    }
    device_opened = true;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file)
{
    device_opened = false;
    module_put(THIS_MODULE);
    return SUCCESS;
}

static ssize_t
device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    if(curPin == NULL)
    {
        printk(KERN_ALERT "No pin to read has been set");
        return -ENODATA;
    }
    sprintf(dev_res_string, "%d", ReadPin(curPin));
    sprintf(buffer, dev_res_string);
    printk(KERN_INFO "%s\n", dev_res_string);
    return 0;
}

static ssize_t
device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
    PIN* pin = NULL;
    result = sscanf(buff, "%c j%d.%d", &operation, &connector, &pin_number);
    if(result != 3)
    {
        printk(KERN_ALERT "Expected 3 arguments, received %d\n", result);
        return -EINVAL;
    }

    pin = getPin(pin_number, connector);
    if(pin == NULL)
    {
        printk(KERN_WARNING "Pin j%d.%d does not exist\n", pin_number, connector);
        return -EINVAL;
    }

    switch(operation)
    {
        case 'h':
            printk(KERN_INFO "High Connector<%d> Pin<%d>\n", connector, pin_number);
            SetPinHigh(pin);
            break;
        case 'l':
            printk(KERN_INFO "Low Connector<%d> Pin<%d>\n", connector, pin_number);
            SetPinLow(pin);
            break;
        case 'r':
            printk(KERN_INFO "Read Connector<%d> Pin<%d>\n", connector, pin_number);
            curPin = pin;
            break;
        default:
            printk(KERN_WARNING "Invalid command: %c\n", operation);
            break;
    }

    return len;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

int __init sysfs_init(void)
{
    int result = 0;

    gpio_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
    if(gpio_obj == NULL)
    {
        printk(KERN_WARNING "%s failed: kobject_create_and_add failed\n",
            sysfs_file);
        return -ENOMEM;
    }

    result = sysfs_create_group(gpio_obj, &attr_group);
    if(result != 0)
    {
        printk(KERN_WARNING "%s failed: sysfs_create_group failed result %d\n",
            sysfs_file, result);
        kobject_put(gpio_obj);
        return -ENOMEM;
    }

    printk(KERN_INFO "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);

    major_number = register_chrdev(213, device_name, &fops);
    if(major_number != 0)
    {
        printk(KERN_ALERT "Registering char device failed with %d\n", major_number);
        return major_number;
    }
    printk(KERN_INFO "Created char device");
    printk(KERN_INFO "Major number: %d", major_number);

    initializePorts();

    return result;
}

void __exit sysfs_exit(void)
{
    kobject_put(gpio_obj);
    printk(KERN_INFO "/sys/kernel/%s/%s removed\n", sysfs_dir, sysfs_file);
    unregister_chrdev(major_number, device_name);
    printk(KERN_INFO "chr device registered\n");
}


module_init(sysfs_init);
module_exit(sysfs_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("f_Vasilis_Josh_Elizabet");
MODULE_DESCRIPTION("GPIO Driver");
