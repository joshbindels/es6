#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Josh");
MODULE_DESCRIPTION("Peek and poke module");

/*
 * Start of virtual addresses for IO devices
 */
#define IO_BASE     0xF0000000

/*
 * This macro relies on fact that for all HW i/o addresses bits 20-23 are 0
 */
#define IO_ADDRESS(x)   (((((x) & 0xff000000) >> 4) | ((x) & 0xfffff)) |\
             IO_BASE)

#define io_p2v(x)   ((void __iomem *) (unsigned long) IO_ADDRESS(x))
#define io_v2p(x)   ((((x) & 0x0ff00000) << 4) | ((x) & 0x000fffff))

#define sysfs_dir "es6"
#define sysfs_file "peek_and_poke"
#define sysfs_max_data_size 1024

//static char sysfs_buffer[sysfs_max_data_size + 1] = "\0";
/*
static ssize_t used_buffer_size = 0;
static char input_address[9];
static char input_number[3];
static char input_operation;
static int index_read;
static int index_write;
static uint32_t* register_ptr;
static uint32_t address_in_hex;
*/

#define RTC_UCOUNT  0x40024000
#define RTC_CTRL    0x40024010

//uint32_t* rtc_ucount;
//uint32_t* rtc_ctrl;

static ssize_t  sysfs_show(struct device *dev,
    struct device_attribute *attr, char *buffer)
{
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n",
        sysfs_dir, sysfs_file);
    /* stop rtc */
    //*rtc_ctrl = (1<<6);
    return sprintf(buffer, "I LIKE CHOCOLATE CAKE\n");
}

static uint32_t hex2int(char *hex) {
    /*
     * Convert string representing hex number into an int
     * source: https://stackoverflow.com/questions/10156409/convert-hex-string-char-to-int
     */
    uint32_t val = 0;
    while (*hex) {
        // get current character then increment
        uint8_t byte = *hex++;
        // transform hex character to the 4bit equivalent number, using the ascii table indexes
        if (byte >= '0' && byte <= '9') byte = byte - '0';
        else if (byte >= 'a' && byte <='f') byte = byte - 'a' + 10;
        else if (byte >= 'A' && byte <='F') byte = byte - 'A' + 10;
        // shift 4 to make space for new digit, and add the 4 bits of the new digit
        val = (val << 4) | (byte & 0xF);
    }
    return val;
}

static ssize_t sysfs_store(struct device* dev,
    struct device_attribute* attr, const char* buffer,
    size_t count)
{
    used_buffer_size = count > sysfs_max_data_size ? sysfs_max_data_size : count;
    char address[9];
    char number[8] = "\0";
    int index_read = 2;
    int index_write = 0;
    uint32_t* register_ptr;
    uint32_t hex;

    input_operation = buffer[0];
    if(buffer[0] == 'r')
    {
        for(index_read=2;index_read<used_buffer_size;index_read++)
        {
            if(index_read<10)
            {
                input_address[index_write++] = buffer[index_read];
            }
            else if(index_read == 10)
            {
                input_address[8] = '\0';
                index_write = 0;
            }
            else if(index_read > 10)
            {
                input_number[index_write++] = buffer[index_read];
            }
        }
        input_number[index_write-1] = '\0';
        printk(KERN_INFO "Address: %s Number: %s) called\n", input_address, input_number);
    }

    hex = hex2int(input_address);
    register_ptr = io_p2v(address_in_hex);

    printk(KERN_INFO "(%s) : (%x)", address, *register_ptr);

    return used_buffer_size;
}

static DEVICE_ATTR(peek_and_poke, S_IWUGO | S_IRUGO
    , sysfs_show, sysfs_store);

static struct attribute* attrs[] = {
    &dev_attr_peek_and_poke.attr,
    NULL
};

static struct attribute_group attr_group = {
    .attrs = attrs,
};

static struct kobject* peek_obj = NULL;

int __init sysfs_init(void)
{
    int result = 0;

    peek_obj = kobject_create_and_add(sysfs_dir, kernel_kobj);
    if(peek_obj == NULL)
    {
        printk(KERN_WARNING
            "%s module failed to load: "
            "kobject_create_and_add failed\n",
            sysfs_file);
        return -ENOMEM;
    }

    result = sysfs_create_group(peek_obj, &attr_group);
    if(result != 0)
    {
        printk(KERN_WARNING
            "%s module failed to load: sysfs_create_group "
            "failed with result %d\n", sysfs_file, result);
        kobject_put(peek_obj);
        return -ENOMEM;
    }

    //rtc_ucount = io_p2v(RTC_UCOUNT);
    //rtc_ctrl = io_p2v(RTC_CTRL);

    printk(KERN_INFO
        "/sys/kernel/%s/%s created\n", sysfs_dir, sysfs_file);
    return result;
}

void __exit sysfs_exit(void)
{
    kobject_put(peek_obj);
    printk(KERN_INFO "/sys/kernel/%s/%s removed\n",
        sysfs_dir, sysfs_file);
}

module_init(sysfs_init);
module_exit(sysfs_exit);
