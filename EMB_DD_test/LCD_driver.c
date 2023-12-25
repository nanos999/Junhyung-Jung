#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/i2c.h>
#include <linux/kernel.h>

#define DRIVER_NAME "i2c_lcd"
#define DRIVER_CLASS "lcdClass"

static struct i2c_adapter *lcd_i2c_adapter = NULL;
static struct i2c_client *lcd_i2c_client = NULL;

/* I2C 1602 LCD Commands */
#define LCD_CMD_CLEAR_DISPLAY 0x01
#define LCD_CMD_RETURN_HOME 0x02
#define LCD_CMD_ENTRY_MODE_SET 0x04
#define LCD_CMD_DISPLAY_CONTROL 0x08
#define LCD_CMD_CURSOR_SHIFT 0x10
#define LCD_CMD_FUNCTION_SET 0x20
#define LCD_CMD_SET_CGRAM_ADDR 0x40
#define LCD_CMD_SET_DDRAM_ADDR 0x80

/* I2C 1602 LCD Function Set Options */
#define LCD_FUNCTION_SET_8BIT 0x10
#define LCD_FUNCTION_SET_4BIT 0x00
#define LCD_FUNCTION_SET_2LINE 0x08
#define LCD_FUNCTION_SET_1LINE 0x00
#define LCD_FUNCTION_SET_5X10DOTS 0x04
#define LCD_FUNCTION_SET_5X8DOTS 0x00

/* I2C 1602 LCD Display Control Options */
#define LCD_DISPLAY_ON 0x04
#define LCD_DISPLAY_OFF 0x00
#define LCD_CURSOR_ON 0x02
#define LCD_CURSOR_OFF 0x00
#define LCD_BLINK_ON 0x01
#define LCD_BLINK_OFF 0x00

/* I2C 1602 LCD Entry Mode Set Options */
#define LCD_ENTRY_INCREMENT 0x02
#define LCD_ENTRY_DECREMENT 0x00
#define LCD_ENTRY_SHIFT_ON 0x01
#define LCD_ENTRY_SHIFT_OFF 0x00

/* I2C 1602 LCD Write Options */
#define LCD_WRITE_RS 0x01
#define LCD_WRITE_DATA 0x40

/**
 * @brief Send a command to the LCD
 *
 * @param cmd The command to be sent
 */
static void lcd_send_command(u8 cmd) {
    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, cmd);
    msleep(5); // Wait for command execution
}

/**
 * @brief Write data to the LCD
 *
 * @param data The data to be written
 * @param rs RS (Register Select) value (1 for data, 0 for command)
 */
static void lcd_write_data(u8 data, u8 rs) {
    u8 high_nibble = ((data & 0xF0) | rs | LCD_WRITE_DATA);
    u8 low_nibble = (((data << 4) & 0xF0) | rs | LCD_WRITE_DATA);

    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, high_nibble);
    msleep(1);
    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, high_nibble | 0x08); // Enable High
    msleep(1);
    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, high_nibble); // Enable Low
    msleep(1);

    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, low_nibble);
    msleep(1);
    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, low_nibble | 0x08); // Enable High
    msleep(1);
    i2c_smbus_write_byte_data(lcd_i2c_client, 0x00, low_nibble); // Enable Low
    msleep(1);
}

/**
 * @brief Write a string to the LCD
 *
 * @param str The string to be written
 */
static void lcd_write_string(const char *str) {
    while (*str != '\0') {
        lcd_write_data(*str, LCD_WRITE_RS);
        str++;
    }
}

/**
 * @brief This function is called when writing to the device file
 */
static ssize_t lcd_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *offs) {
    char *data;
    int ret;

    data = kmalloc(count, GFP_KERNEL);
    if (!data) {
        return -ENOMEM;
    }

    ret = copy_from_user(data, user_buffer, count);
    if (ret) {
        kfree(data);
        return -EFAULT;
    }

    /* Write the data to the LCD */
    lcd_write_string(data);

    kfree(data);

    return count;
}

/**
 * @brief This function is called, when the device file is opened
 */
static int lcd_open(struct inode *deviceFile, struct file *instance) {
    printk("LCD Driver - Open was called\n");
    return 0;
}

/**
 * @brief This function is called, when the device file is close
 */
static int lcd_close(struct inode *deviceFile, struct file *instance) {
    printk("LCD Driver - Close was called\n");
    return 0;
}

/* Map the file operations */
static struct file_operations lcd_fops = {
    .owner = THIS_MODULE,
    .open = lcd_open,
    .release = lcd_close,
    .write = lcd_write,
};

/**
 * @brief function, which is called after loading module to kernel, do initialization there
 */
static int __init lcd_init_module(void) {
    int ret = -1;
    printk("LCD Driver - Hello Kernel\n");

    /* Allocate Device Nr */
    if (alloc_chrdev_region(&lcd_device_nr, 0, 1, DRIVER_NAME) < 0) {
        printk("Device Nr. could not be allocated!\n");
        return -ENOMEM;
    }
    printk("LCD Driver - Device Nr %d was registered\n", lcd_device_nr);

    /* Create Device Class */
    if ((lcd_class = class_create(THIS_MODULE, DRIVER_CLASS)) == NULL) {
        printk("Device Class can not be created!\n");
        goto ClassError;
    }

    /* Create Device file */
    if (device_create(lcd_class, NULL, lcd_device_nr, NULL, DRIVER_NAME) == NULL) {
        printk("Can not create device file!\n");
        goto FileError;
    }

    /* Initialize Device file */
    cdev_init(&lcd_device, &lcd_fops);

    /* register device to kernel */
    if (cdev_add(&lcd_device, lcd_device_nr, 1) == -1) {
        printk("Registering of device to kernel failed!\n");
        goto KernelError;
    }

    lcd_i2c_adapter = i2c_get_adapter(1); // Use the appropriate I2C bus number

    if (lcd_i2c_adapter != NULL) {
        lcd_i2c_client = i2c_new_device(lcd_i2c_adapter, &(struct i2c_board_info){I2C_BOARD_INFO(DRIVER_NAME, 0x27)});
        if (lcd_i2c_client != NULL) {
            lcd_send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT);
            lcd_send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT);
            lcd_send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT);
            lcd_send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_4BIT);
            lcd_send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_4BIT | LCD_FUNCTION_SET_2LINE);
            lcd_send_command(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF);
            lcd_send_command(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF);

            ret = 0;
        }
        i2c_put_adapter(lcd_i2c_adapter);
    }
    printk("I2C 1602 LCD Driver added!\n");

    return ret;

KernelError:
    device_destroy(lcd_class, lcd_device_nr);
FileError:
    class_destroy(lcd_class);
ClassError:
    unregister_chrdev_region(lcd_device_nr, 1);
    return ret;
}

/**
 * @brief function, which is called when removing module from kernel
 * free allocated resources
 */
static void __exit lcd_exit_module(void) {
    printk("LCD Driver - Goodbye, Kernel!\n");
    i2c_unregister_device(lcd_i2c_client);
    cdev_del(&lcd_device);
    device_destroy(lcd_class, lcd_device_nr);
    class_destroy(lcd_class);
    unregister_chrdev_region(lcd_device_nr, 1);
}

module_init(lcd_init_module);
module_exit(lcd_exit_module);

MODULE_AUTHOR("Your Name");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("I2C 1602 LCD Driver");
