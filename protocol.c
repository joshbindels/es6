#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <mach/hardware.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Josh");
MODULE_DESCRIPTION("Peek and poke module");


#define sysfs_dir "es6"
#define sysfs_file "peek_and_poke"
#define sysfs_max_data_size 1024
#define register_size 4

static char sysfs_buffer[sysfs_max_data_size + 1] = "\0";
static ssize_t used_buffer_size = 0;
//static char input_address[9];
//static char input_number[3];
//static char input_operation;
static int index_read;
static int index_write = 0;
static uint32_t* register_ptr;
static char register_value_string[register_size*2 + 1] = "\0";
static uint32_t address_in_hex;

static char input_operation;
static uint32_t input_address;
static uint32_t input_number;

#define RTC_UCOUNT  0x40024000
#define RTC_CTRL    0x40024010

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

static ssize_t  sysfs_show(struct device *dev,
    struct device_attribute *attr, char *buffer)
{
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n",
        sysfs_dir, sysfs_file);
    return sprintf(buffer, "%s", sysfs_buffer);
}

/*
static void parse_input(const char* buffer)
{
    index_write = 0;
    input_operation = buffer[0];
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
}
*/

static ssize_t sysfs_store(struct device* dev,
    struct device_attribute* attr, const char* buffer,
    size_t count)
{
    used_buffer_size = count > sysfs_max_data_size ? sysfs_max_data_size : count;

    //parse_input(buffer);
    sscanf(buffer, "%c %x %x", &input_operation, &input_address, &input_number);
    //address_in_hex = hex2int(input_address);
    register_ptr = io_p2v(input_address);

    if(input_operation == 'r')
    {
        printk(KERN_INFO "Reading %x bytes from address: %x\n",
            input_number, input_address);

        sprintf(register_value_string, "%x", *register_ptr);

        memcpy(sysfs_buffer, register_value_string, register_size*2);
        sysfs_buffer[register_size*2 + 1] = '\0';
    }
    else if(input_operation == 'w')
    {
        printk(KERN_INFO "Writing %x to address (%x)\n", input_number, input_address);
        *register_ptr = input_number;
    }
    else
    {
        printk(KERN_WARNING "Invalid operation: %c\n", input_operation);
    }
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
