#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/delay.h>

/* Meta Information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A simple buzzer driver for generating sound");

/* Variables for device and device class */
static dev_t buzzer_dev_number;
static struct class *buzzer_class;
static struct cdev buzzer_cdev;

#define DRIVER_NAME "buzzer"
#define DRIVER_CLASS "BuzzerModuleClass"

#define YOUR_BUZZER_PIN 19  // Replace with the actual GPIO pin number

/* Buzzer state: 0 - off, 1 - on */
static int buzzer_state = 0;

/**
 * @brief Generate a simple beep sound using the buzzer
 */
static void generate_beep(void) {
    gpio_set_value(YOUR_BUZZER_PIN, 1);  // Set GPIO pin high
    msleep(500);  // Delay for 500 milliseconds (adjust as needed)
    gpio_set_value(YOUR_BUZZER_PIN, 0);  // Set GPIO pin low
}

/**
 * @brief This function is called when the device file is opened
 */
static int buzzer_open(struct inode *inode, struct file *file) {
    printk("Buzzer - Open was called!\n");
    return 0;
}

/**
 * @brief This function is called when the device file is closed
 */
static int buzzer_release(struct inode *inode, struct file *file) {
    printk("Buzzer - Close was called!\n");
    return 0;
}

/**
 * @brief This function is called when data is written to the device
 */
static ssize_t buzzer_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos) {
    char data;

    if (copy_from_user(&data, buf, 1)) {
        return -EFAULT;
    }

    if (data == '0') {
        // Turn off the buzzer
        gpio_set_value(YOUR_BUZZER_PIN, 0);
        buzzer_state = 0;
    } else if (data == '1') {
        // Turn on the buzzer
        gpio_set_value(YOUR_BUZZER_PIN, 1);
        buzzer_state = 1;
    }

    return 1;
}

static struct file_operations buzzer_fops = {
    .owner = THIS_MODULE,
    .open = buzzer_open,
    .release = buzzer_release,
    .write = buzzer_write,
};

static int __init buzzer_init(void) {
    printk("Hello, Buzzer Kernel!\n");

    // Allocate a device number
    if (alloc_chrdev_region(&buzzer_dev_number, 0, 1, DRIVER_NAME) < 0) {
        printk("Device number allocation failed!\n");
        return -1;
    }
    printk("Buzzer - Device Major: %d, Minor: %d was registered!\n", MAJOR(buzzer_dev_number), MINOR(buzzer_dev_number));

    // Create device class
    if ((buzzer_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        printk("Device class creation failed!\n");
        goto class_error;
    }

    // Create device file
    if (device_create(buzzer_class, NULL, buzzer_dev_number, NULL, DRIVER_NAME) == NULL) {
        printk("Device creation failed!\n");
        goto file_error;
    }

    // Initialize cdev
    cdev_init(&buzzer_cdev, &buzzer_fops);

    // Add cdev to the kernel
    if (cdev_add(&buzzer_cdev, buzzer_dev_number, 1) == -1) {
        printk("Cdev registration failed!\n");
        goto cdev_error;
    }

    // Initialize GPIO pin (replace YOUR_BUZZER_PIN with the actual GPIO pin number)
    if (gpio_request(YOUR_BUZZER_PIN, "buzzer-gpio")) {
        printk("Cannot allocate GPIO pin %d\n", YOUR_BUZZER_PIN);
        goto gpio_error;
    }

    // Set GPIO pin direction to output
    if (gpio_direction_output(YOUR_BUZZER_PIN, 0)) {
        printk("Cannot set GPIO pin %d to output!\n", YOUR_BUZZER_PIN);
        goto gpio_error;
    }

    return 0;

gpio_error:
    gpio_free(YOUR_BUZZER_PIN);
cdev_error:
    device_destroy(buzzer_class, buzzer_dev_number);
file_error:
    class_destroy(buzzer_class);
class_error:
    unregister_chrdev_region(buzzer_dev_number, 1);
    return -1;
}

static void __exit buzzer_exit(void) {
    gpio_free(YOUR_BUZZER_PIN);
    cdev_del(&buzzer_cdev);
    device_destroy(buzzer_class, buzzer_dev_number);
    class_destroy(buzzer_class);
    unregister_chrdev_region(buzzer_dev_number, 1);
    printk("Goodbye, Buzzer Kernel!\n");
}

module_init(buzzer_init);
module_exit(buzzer_exit);
