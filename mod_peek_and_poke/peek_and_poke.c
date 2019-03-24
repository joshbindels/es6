#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <mach/hardware.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("f_Vasilis_Josh_Elizabet");
MODULE_DESCRIPTION("Peek and poke module for reading and writing values to registers using their physical address.");

#define sysfs_dir "es6"
#define sysfs_file "peek_and_poke"
#define register_size 4
#define max_buffer_size 1024

/* input parameters */
static char input_operation = 0;
static uint32_t input_address = 0;
static uint32_t input_number = 0;

static int result = 0;
static char result_string[max_buffer_size] = "";

/* register */
static uint32_t* register_ptr;
int OFFSET = 0;

static ssize_t  sysfs_show(struct device *dev,
    struct device_attribute *attr, char *buffer)
{
    result_string[0] = '\0'; // reset result string
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n", sysfs_dir, sysfs_file);
    for (OFFSET=0; OFFSET<input_number; OFFSET++)
    {
        register_ptr = io_p2v(input_address + OFFSET * register_size);
        sprintf(result_string, "%s 0x%lx", result_string, *register_ptr);
    }
    return sprintf(buffer, result_string);
}

static ssize_t
sysfs_store(struct device* dev, struct device_attribute* attr, const char* buffer, size_t count)
{
    result = sscanf(buffer, "%c %x %x", &input_operation, &input_address, &input_number);
    if(result != 3)
    {
        printk(KERN_WARNING "Invalid number of parameters: %d\n", result);
        return -EINVAL;
    }

    if(input_operation == 'r')
    {
        printk(KERN_INFO "Received read request of %x bytes from address %x",
            input_number, input_address);
    }
    else if(input_operation == 'w')
    {
        printk(KERN_INFO "Writing %x to address (%x)\n", input_number, input_address);
        register_ptr = io_p2v(input_address);
        *register_ptr = input_number;
    }
    else
    {
        printk(KERN_WARNING "Invalid operation: %c\n", input_operation);
        return -EINVAL;
    }
    return count;
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
