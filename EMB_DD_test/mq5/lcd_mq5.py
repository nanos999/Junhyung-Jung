import smbus
import time
import I2C_LCD_driver

# I2C 설정
bus = smbus.SMBus(1)  # I2C 버스 1 사용
AIN0 = 0x48  # 가스 센서의 I2C 주소

# LCD 설정
lcd = I2C_LCD_driver.lcd()

try:
    # 초기 센서 값 읽기
    first = bus.read_byte(AIN0)

    while True:
        # 현재 날짜와 시간 가져오기
        current_date = time.strftime("%m/%d")
        current_time = time.strftime("%H:%M:%S")

        # 가스 센서 값 읽기
        gas_value = bus.read_byte(AIN0)
        print("Gas value: {}".format(gas_value))

        # LCD에 날짜와 시간 표시
        lcd.lcd_display_string(f"{current_date} {current_time}", 1)

        # 가스 감지 여부 판단 및 표시
        if gas_value > 30:  # 'threshold'는 적절한 값으로 설정하세요
            lcd.lcd_display_string(f"Gas Level: {gas_value}", 2)
        else:
            lcd.lcd_display_string("No gas.", 2)

        time.sleep(1)  # 1초 대기
        lcd.lcd_clear()

except KeyboardInterrupt:
    print("Keyboard interrupt occurred")



