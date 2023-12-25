#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "mq5_driver"
#define CLASS_NAME  "mq5"
#define GAS_SENSOR_PIN 23

static int    majorNumber;
static struct class*  mq5Class  = NULL;
static struct device* mq5Device = NULL;
static struct cdev c_dev;
static dev_t dev_num;

// Prototype functions for the character driver
static int     dev_open(struct inode *, struct file *);
static int     dev_release(struct inode *, struct file *);
static ssize_t dev_read(struct file *, char *, size_t, loff_t *);

static struct file_operations fops =
{
   .open = dev_open,
   .read = dev_read,
   .release = dev_release,
};

static int __init mq5_driver_init(void) {
    printk(KERN_INFO "MQ5: Initializing the MQ5 LKM\n");

    // Try to dynamically allocate a major number for the device
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
    {
        return -1;
    }
    majorNumber = MAJOR(dev_num);

    // Register the device class
    mq5Class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(mq5Class))
    {
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(mq5Class);
    }

    // Register the device driver
    mq5Device = device_create(mq5Class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(mq5Device))
    {
        class_destroy(mq5Class);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(mq5Device);
    }

    cdev_init(&c_dev, &fops);
    if (cdev_add(&c_dev, dev_num, 1) == -1)
    {
        device_destroy(mq5Class, dev_num);
        class_destroy(mq5Class);
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    printk(KERN_INFO "MQ5: device class created correctly\n");

    // Initialize GPIO pin
    gpio_request(GAS_SENSOR_PIN, "sysfs");
    gpio_direction_input(GAS_SENSOR_PIN);
    gpio_export(GAS_SENSOR_PIN, false);

    return 0;
}

static void __exit mq5_driver_exit(void) {
    cdev_del(&c_dev);
    device_destroy(mq5Class, dev_num);
    class_destroy(mq5Class);
    unregister_chrdev_region(dev_num, 1);
    gpio_unexport(GAS_SENSOR_PIN);
    gpio_free(GAS_SENSOR_PIN);

    printk(KERN_INFO "MQ5: Goodbye from the LKM!\n");
}

static int dev_open(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "MQ5: Device has been opened\n");
   return 0;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    int gas_value = gpio_get_value(GAS_SENSOR_PIN);
    char message[20] = {0};
    snprintf(message, sizeof(message), "%d\n", gas_value);

    // copy_to_user has the format ( * to, *from, size) and returns 0 on success
    error_count = copy_to_user(buffer, message, strlen(message));

    if (error_count==0) {
        return (size_t)(strlen(message));
    }
    else {
        printk(KERN_INFO "MQ5: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;              // Failed -- return a bad address message (i.e. -14)
    }
}

static int dev_release(struct inode *inodep, struct file *filep) {
   printk(KERN_INFO "MQ5: Device successfully closed\n");
   return 0;
}

module_init(mq5_driver_init);
module_exit(mq5_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("Linux Char Driver for MQ5 Gas Sensor");
MODULE_VERSION("0.1");
