import smbus
import time

bus = smbus.SMBus(1)  # I2C 버스 1 사용
AIN0 = 0x48  # 센서의 I2C 주소

try:
    # 초기 센서 값 읽기
    first = bus.read_byte(AIN0)

    while True:
        # 센서 값 읽기
        gas_value = bus.read_byte(AIN0)
        print("가스 값: {}".format(gas_value))

        time.sleep(1)  # 1초 대기

except KeyboardInterrupt:
    print("키보드 인터럽트 발생")
