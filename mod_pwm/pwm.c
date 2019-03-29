#include <asm/uaccess.h> /* for copy_to_user */
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <mach/hardware.h>

#include "pwm.h"

static int map_duty(int* val)
{
    if((*val <= 0) | (*val > 100))
    {
        return -EINVAL;
    }
    return (*val * 255) / 100;
}

static int map_frequency(int* val)
{
    if((*val <= 0) | (*val > PWM_CLOCK_FREQ))
    {
        return -EINVAL;
    }
    return (PWM_CLOCK_FREQ / *val) / 256;
}


static int device_open(struct inode* inode, struct file* file)
{
    if(device_opened)
    {
        return -EBUSY;
    }
    device_opened = true;
    how_often_opened++;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode* inode, struct file* file)
{
    device_opened = false;
    module_put(THIS_MODULE);
    file->private_data = NULL;
    return 0;
}

static ssize_t
device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    const int MaxSize = length < 50 ? length : 50;
    char msg[MaxSize];
    int msgLength = 0;
    int bytesRemaining = 0;

    minor_number = iminor(filp->f_path.dentry->d_inode);

    if(*offset != 0)
    {
        return 0;
    }

    switch(minor_number)
    {
        case MIN_PWM1_FREQ:
        {
            msgLength = snprintf(msg, MaxSize, "Frequency: %d\n", pwm1_frequency);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        case MIN_PWM1_DUTY:
        {
            msgLength = snprintf(msg, MaxSize, "Duty: %d\n", pwm1_duty);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        case MIN_PWM1_ENABLE:
        {
            msgLength = snprintf(msg, MaxSize, "Enabled: %d\n", pwm1_enabled);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        case MIN_PWM2_FREQ:
        {
            msgLength = snprintf(msg, MaxSize, "Frequency: %d\n", pwm2_frequency);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        case MIN_PWM2_DUTY:
        {
            msgLength = snprintf(msg, MaxSize, "Duty: %d\n", pwm2_duty);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        case MIN_PWM2_ENABLE:
        {
            msgLength = snprintf(msg, MaxSize, "Enabled: %d\n", pwm2_enabled);
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
        default:
        {
            msgLength = snprintf(msg, MaxSize, "Invalid read \n");
            bytesRemaining = copy_to_user(buffer, msg, msgLength);
            *offset += msgLength - bytesRemaining;
            break;
        }
    }

    return msgLength - bytesRemaining;

}

static ssize_t
device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
    result = sscanf(buff, "%d", &input_val);
    if (result != 1)
    {
        printk(KERN_ALERT "Write takes 1 input value, received %d", result);
        return -EINVAL;
    }

    if(input_val < 0 && input_val > 255)
    {
        printk(KERN_ALERT "Invalid value: %d, number should be between 0 and 255", input_val);
        return -EINVAL;
    }

    minor_number = iminor(filp->f_path.dentry->d_inode);

    switch(minor_number)
    {
        case MIN_PWM1_FREQ:
        {
            pwm1_frequency = input_val;
            adj_input_val = map_frequency(&pwm1_frequency);
            if(adj_input_val < 0)
            {
                return -EINVAL;
            }
            *PWM1_PTR |= (adj_input_val << PWM_RELOADV);
            printk(KERN_INFO "Writing freq<%d Hz> to pwm1\n", pwm1_frequency);
            break;
        }
        case MIN_PWM1_DUTY:
        {
            pwm1_duty = input_val;
            adj_input_val = map_duty(&pwm1_duty);
            if(adj_input_val < 0)
            {
                return -EINVAL;
            }
            *PWM1_PTR |= (adj_input_val << PWM_DUTY);
            printk(KERN_INFO "Writing duty<%d%%> to pwm1\n", pwm1_duty);
            break;
        }
        case MIN_PWM1_ENABLE:
        {
            pwm1_enabled = input_val;
            *PWM1_PTR = ((*PWM1_PTR) & ~(1<<PWM_EN)) ^ (pwm1_enabled<<PWM_EN);
            printk(KERN_INFO "Enabling pwm1\n");
            break;
        }
        case MIN_PWM2_FREQ:
        {
            pwm2_frequency = input_val;
            adj_input_val = map_frequency(&pwm2_frequency);
            if(adj_input_val < 0)
            {
                return -EINVAL;
            }
            *PWM2_PTR |= (adj_input_val << PWM_RELOADV);
            printk(KERN_INFO "Writing freq<%d Hz> to pwm2\n", pwm2_frequency);
            break;
        }
        case MIN_PWM2_DUTY:
        {
            pwm2_duty = input_val;
            adj_input_val = map_duty(&pwm2_duty);
            if(adj_input_val < 0)
            {
                return -EINVAL;
            }
            *PWM2_PTR |= (adj_input_val << PWM_DUTY);
            printk(KERN_INFO "Writing duty<%d%%> to pwm2\n", pwm2_duty);
            break;
        }
        case MIN_PWM2_ENABLE:
        {
            pwm2_enabled = input_val;
            *PWM2_PTR = ((*PWM2_PTR) & ~(1<<PWM_EN)) ^ (pwm2_enabled<<PWM_EN);
            printk(KERN_INFO "Enabling pwm2\n");
            break;
        }
        default:
        {
            printk(KERN_INFO "device write called with minor number 3");
            break;
        }
    }
    return len;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

int init_module(void)
{
    major_number = register_chrdev(0, DEVICE_NAME, &fops);

    if(major_number == 0)
    {
        printk(
            KERN_ALERT "Registering char device failed with %d\n",
            major_number
        );
        return major_number;
    }

    printk(
        KERN_INFO "I was assigned major number %d. To talk to\n",
        major_number
    );
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'mknod /dev/%s c %d 0'.\n", DEVICE_NAME, major_number);
    printk(KERN_INFO "the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    PWM1_PTR = (uint32_t*)(io_p2v(PWM1_ADDR));
    PWM2_PTR = (uint32_t*)(io_p2v(PWM2_ADDR));
    *(uint32_t*)(io_p2v(PWM_CLOCK)) = PWM_CLOCK_VAL;
    *(uint32_t*)(io_p2v(LCD_CONF)) = LCD_CONF_VAL;

    return SUCCESS;
}

void cleanup_module(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("group"); //TODO: enter group name
MODULE_DESCRIPTION("pwm module es6");
