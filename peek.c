#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
//#include <linux/io.h>

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

#define RTC_UCOUNT  0x40024000
#define RTC_CTRL    0x40024010

uint32_t* rtc_ucount = io_p2v(RTC_UCOUNT);
uint32_t* rtc_ctrl = io_p2v(RTC_CTRL);

static ssize_t  sysfs_show(struct device *dev,
    struct device_attribute *attr, char *buffer)
{
    printk(KERN_INFO "sysfile_read (/sys/kernel/%s/%s) called\n",
        sysfs_dir, sysfs_file);
    /* stop rtc */
    *rtc_ctrl = (1<<6);
    return sprintf(buffer, "rtc_ucount: %x\n", *rtc_ucount);
}

static DEVICE_ATTR(peek_and_poke, S_IRUGO, sysfs_show, NULL);

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
