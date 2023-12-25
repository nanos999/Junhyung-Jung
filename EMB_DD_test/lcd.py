import smbus
import time

# I2C LCD Address (adjust this based on your actual LCD address)
LCD_ADDR = 0x27

# I2C bus number (use the correct I2C bus number)
I2C_BUS = 1

bus = smbus.SMBus(I2C_BUS)

def lcd_byte(bits, mode):
    bits_high = mode | (bits & 0xF0) | 0x08
    bits_low = mode | ((bits << 4) & 0xF0) | 0x08

    bus.write_byte(LCD_ADDR, bits_high)
    lcd_toggle_enable(bits_high)

    bus.write_byte(LCD_ADDR, bits_low)
    lcd_toggle_enable(bits_low)

def lcd_toggle_enable(bits):
    time.sleep(0.0005)
    bus.write_byte(LCD_ADDR, (bits | 0x04))
    time.sleep(0.0005)
    bus.write_byte(LCD_ADDR, (bits & ~0x04))
    time.sleep(0.0005)

def lcd_string(message, line):
    message = message.ljust(16, " ")
    if line == 1:
        lcd_byte(0x80, 0)
    elif line == 2:
        lcd_byte(0xC0, 0)

    for i in range(16):
        lcd_byte(ord(message[i]), 1)

def lcd_clear():
    lcd_byte(0x01, 0)

# Test 1
while True:
    lcd_string("Developer", 1)
    lcd_string("Dreamming!!", 2)
    time.sleep(1)
    lcd_clear()
    time.sleep(1)
