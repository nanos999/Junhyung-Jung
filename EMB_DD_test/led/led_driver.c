#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>

/* Meta Information */ 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Johannes 4 GNU/Linux");
MODULE_DESCRIPTION("A simple gpio driver for setting a LED");

/* Variables for device and device class */ 
static dev_t my_device_nr;
static struct class *my_class; 
static struct cdev my_device;

#define DRIVER_NAME "EMB_RGB_LED"
#define DRIVER_CLASS "MyModuleClass"

/* GPIO 핀 번호 정의 */
#define RED_LED 16
#define BLUE_LED 20
#define GREEN_LED 21

/**
* @brief Read data out of the buffer
*/
static ssize_t driver_read(struct file *File, char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char tmp;

/* Get amount of data to copy */
    to_copy = min(count, sizeof(tmp));
/* Copy data to user */
    not_copied = copy_to_user(user_buffer, &tmp, to_copy);
/* Calculate data */
    delta = to_copy - not_copied;
    return delta;
}

static ssize_t driver_write(struct file *File, const char *user_buffer, size_t count, loff_t *offs) {
    int to_copy, not_copied, delta;
    char value;
/* Get amount of data to copy */
    to_copy = min(count, sizeof(value));
/* Copy data to user */ 
    not_copied = copy_from_user(&value, user_buffer, to_copy);
/* Setting the LED */
switch(value) {
    case '0':
        gpio_set_value(RED_LED, 0); break;
    case '1':
        gpio_set_value(RED_LED, 1); break;
    case '2':
        gpio_set_value(BLUE_LED, 0); break;
    case '3':
        gpio_set_value(BLUE_LED, 1); break;
    case '4':
        gpio_set_value(GREEN_LED, 0); break;
    case '5':
        gpio_set_value(GREEN_LED, 1); break;

    default:
        printk("Invalid Input!\n");
        break;
    }
/* Calculate data */
    delta = to_copy - not_copied;
    return delta;
}

/**
* @brief This function is called, when the device_file is opened
*/
static int driver_open(struct inode *device_file, struct file *instance) {
    printk("led_button - open was called!\n");
    return 0;
}

/**
* @brief This function is called, when the device_file is opened
*/
static int driver_close(struct inode *device_file, struct file *instance) {
    printk("led_button - close was called!\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = driver_read,
    .write = driver_write,
    .open = driver_open,
    .release = driver_close
};

static int __init ModuleInit(void) {
    printk("Hello, Kernel!\n");
/* Allocate a device nr */
    if (alloc_chrdev_region(&my_device_nr, 0, 1, DRIVER_NAME) < 0) {
        printk("Device Nr. could not be allocated!\n");
        return -1;
    }
    printk("read_write - Device Nr. Major: %d, Minor: %d was registered!\n", my_device_nr >> 20, my_device_nr & 0xfffff);
/* Create device class */
    if ((my_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        printk("Device class cannot be created!\n");
        goto ClassError;
    }
/* Create device_file */
    if (device_create(my_class, NULL, my_device_nr, NULL, DRIVER_NAME) == NULL) {
        printk("Cannot create device_file!\n");
        goto FileError;
    }
/* Initialize device_file */ 
    cdev_init(&my_device, &fops);
/* Registering device to kernel */ 
    if (cdev_add(&my_device, my_device_nr, 1) == -1) {
        printk("Registering of device to kernel failed!\n");
        goto AddError;
    }
/* GPIO 16 init */ 
    if (gpio_request(16, "rpi-gpio-16")) {
        printk("Cannot allocate GPIO 16\n");
        goto AddError;
    }
/* Set GPIO 16 direction */
    if (gpio_direction_output(16, 0)) {
        printk("Cannot set GPIO 16 to output!\n");
        goto Gpio16Error;
    }
/* GPIO 20 init */ 
    if (gpio_request(20, "rpi-gpio-20")) {
        printk("Cannot allocate GPIO 20\n");
        goto AddError;
    }
/* Set GPIO 20 direction */
    if (gpio_direction_output(20, 0)) {
        printk("Cannot set GPIO 20 to output!\n");
        goto Gpio20Error;
    }
/* GPIO 21 init */ 
    if (gpio_request(21, "rpi-gpio-21")) {
        printk("Cannot allocate GPIO 21\n");
        goto AddError;
    }
/* Set GPIO 21 direction */
    if (gpio_direction_output(21, 0)) {
        printk("Cannot set GPIO 21 to output!\n");
        goto Gpio21Error;
    }
    

    return 0;

Gpio16Error:
    gpio_free(16);
Gpio20Error:
    gpio_free(20);
Gpio21Error:
    gpio_free(21);
AddError:
    device_destroy(my_class, my_device_nr);
FileError:
    class_destroy(my_class);
ClassError:
    unregister_chrdev_region(my_device_nr, 1);
    return -1;
}

static void __exit ModuleExit(void) {
    gpio_set_value(16, 0);
    gpio_set_value(20, 0); 
    gpio_set_value(21, 0);  
    gpio_free(16);
    gpio_free(BLUE_LED);
    gpio_free(GREEN_LED);

    cdev_del(&my_device);
    device_destroy(my_class, my_device_nr);
    class_destroy(my_class); 
    unregister_chrdev_region(my_device_nr, 1); 
    printk("Goodbye, Kernel\n");
}

module_init(ModuleInit); 
module_exit(ModuleExit);
