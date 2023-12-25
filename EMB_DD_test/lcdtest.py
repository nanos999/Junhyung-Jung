import smbus
import time

# Define I2C address of the LCD
LCD_ADDRESS = 0x27

# Define LCD commands
LCD_CMD_CLEAR_DISPLAY = 0x01
LCD_CMD_RETURN_HOME = 0x02
LCD_CMD_ENTRY_MODE_SET = 0x04
LCD_CMD_DISPLAY_CONTROL = 0x08
LCD_CMD_FUNCTION_SET = 0x20

# Define LCD Function Set Options
LCD_FUNCTION_SET_8BIT = 0x10
LCD_FUNCTION_SET_4BIT = 0x00
LCD_FUNCTION_SET_2LINE = 0x08
LCD_FUNCTION_SET_1LINE = 0x00
LCD_FUNCTION_SET_5X10DOTS = 0x04
LCD_FUNCTION_SET_5X8DOTS = 0x00

# Define LCD Display Control Options
LCD_DISPLAY_ON = 0x04
LCD_DISPLAY_OFF = 0x00
LCD_CURSOR_ON = 0x02
LCD_CURSOR_OFF = 0x00
LCD_BLINK_ON = 0x01
LCD_BLINK_OFF = 0x00

# Define LCD Entry Mode Set Options
LCD_ENTRY_INCREMENT = 0x02
LCD_ENTRY_DECREMENT = 0x00
LCD_ENTRY_SHIFT_ON = 0x01
LCD_ENTRY_SHIFT_OFF = 0x00

# Define LCD Write Options
LCD_WRITE_RS = 0x01
LCD_WRITE_DATA = 0x40

class I2CLCD:
    def __init__(self, bus=1, address=LCD_ADDRESS):
        self.bus = smbus.SMBus(bus)
        self.address = address
        self.init_lcd()

    def write_byte(self, byte, rs):
        high_nibble = ((byte & 0xF0) | rs | LCD_WRITE_DATA)
        low_nibble = (((byte << 4) & 0xF0) | rs | LCD_WRITE_DATA)

        self.bus.write_byte(self.address, high_nibble)
        time.sleep(0.0001)
        self.bus.write_byte(self.address, high_nibble | 0x08)  # Enable High
        time.sleep(0.0001)
        self.bus.write_byte(self.address, high_nibble)  # Enable Low
        time.sleep(0.0001)

        self.bus.write_byte(self.address, low_nibble)
        time.sleep(0.0001)
        self.bus.write_byte(self.address, low_nibble | 0x08)  # Enable High
        time.sleep(0.0001)
        self.bus.write_byte(self.address, low_nibble)  # Enable Low
        time.sleep(0.0001)

    def send_command(self, command):
        self.write_byte(command, 0)

    def send_data(self, data):
        self.write_byte(data, LCD_WRITE_RS)

    def init_lcd(self):
        self.send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT)
        self.send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT)
        self.send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_8BIT)
        self.send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_4BIT)
        self.send_command(LCD_CMD_FUNCTION_SET | LCD_FUNCTION_SET_4BIT | LCD_FUNCTION_SET_2LINE)
        self.send_command(LCD_CMD_DISPLAY_CONTROL | LCD_DISPLAY_ON | LCD_CURSOR_OFF | LCD_BLINK_OFF)
        self.send_command(LCD_CMD_ENTRY_MODE_SET | LCD_ENTRY_INCREMENT | LCD_ENTRY_SHIFT_OFF)

    def clear_lcd(self):
        self.send_command(LCD_CMD_CLEAR_DISPLAY)
        time.sleep(0.002)

    def return_home(self):
        self.send_command(LCD_CMD_RETURN_HOME)
        time.sleep(0.002)

    def display_string(self, text):
        self.clear_lcd()
        for char in text:
            self.send_data(ord(char))

# Example of using the I2CLCD class
if __name__ == "__main__":
    lcd = I2CLCD()

    lcd.display_string("Hello, LCD!")

    time.sleep(3)  # Display for 3 seconds

    lcd.clear_lcd()
