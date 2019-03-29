#include <asm/uaccess.h> /* for copy_to_user */
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/module.h>

#define SUCCESS 0
#define DEVICE_NAME "pwm"

static bool device_opened = false;
static int how_often_opened = 0;

static int major_number = 0;
static int minor_number = 0;

/*
 * minor number : node
 * 0 : /dev/pwm_freq
 * 1 : /dev/pwm_duty
 * 2 : /dev/pwm_enabled
 */
static int pwm_enabled = 0;
static int pwm_frequency = 0;
static int pwm_duty = 0;

static int result = 0;
static int input_val = 0;


static int device_open(struct inode* inode, struct file* file)
{
    printk(KERN_INFO "device open() called");
    printk(KERN_INFO "Major number: %u", MAJOR(inode->i_rdev));
    printk(KERN_INFO "Minor number: %u", MINOR(inode->i_rdev));

    //minor_number = MINOR(inode->i_rdev);
    //file->private_data = &minor_number;
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
    printk(KERN_INFO "device_released() called");
    device_opened = false;
    module_put(THIS_MODULE);
    file->private_data = NULL;
    return 0;
}

static ssize_t
device_read(struct file* filp, char* buffer, size_t length, loff_t* offset)
{
    //printk(KERN_INFO "device_read() called on minor number: %u", *(unsigned int*)(filp->private_data));
    //printk(KERN_INFO "device_read() called on minor number: %u", iminor(filp->f_path.dentry->d_inode));
    /*
    const int MaxSize = length < 50 ? length : 50;
    char msg[MaxSize];
    int msgLength = 0;
    int bytesRemaining = 0;

    if(*offset != 0)
    {
        return 0;
    }

    msgLength = snprintf( //TODO: look up snprintf
        msg, MaxSize, "I have been opened %d times!\n", how_often_opened
    );

    bytesRemaining = copy_to_user(buffer, msg, msgLength);

    *offset += msgLength - bytesRemaining;
    return msgLength - bytesRemaining;
    */
    switch(iminor(filp->f_path.dentry->d_inode))
    {
        case 0:
        {
            printk(KERN_INFO "device read called with minor number 0");
            printk(KERN_INFO "PWM_FREQ: %d", pwm_frequency);
            break;
        }
        case 1:
        {
            printk(KERN_INFO "device read called with minor number 1");
            printk(KERN_INFO "PWM_DUTY: %d", pwm_duty);
            break;
        }
        case 2:
        {
            printk(KERN_INFO "device read called with minor number 2");
            printk(KERN_INFO "PWM_ENABLED: %d", pwm_enabled);
            break;
        }
        default:
        {
            printk(KERN_INFO "device read called with minor number 3");
            break;
        }
    }
    return length;

}

static ssize_t
device_write(struct file* filp, const char* buff, size_t len, loff_t* off)
{
    result = sscanf(buff, "%d", &input_val);
    if (result != 1)
    {
        printk(KERN_ALERT "Invalid write command");
        return -EINVAL;
    }
    switch(iminor(filp->f_path.dentry->d_inode))
    {
        case 0:
        {
            printk(KERN_INFO "device write called with minor number 0");
            pwm_frequency = input_val;
            break;
        }
        case 1:
        {
            printk(KERN_INFO "device write called with minor number 1");
            pwm_duty = input_val;
            break;
        }
        case 2:
        {
            printk(KERN_INFO "device write called with minor number 2");
            pwm_enabled = input_val;
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
    return SUCCESS;
}

void cleanup_module(void)
{
    unregister_chrdev(major_number, DEVICE_NAME);
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("group"); //TODO: enter group name
MODULE_DESCRIPTION("pwm module es6");
