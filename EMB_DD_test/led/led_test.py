import os
import time

# 각 색상의 GPIO 핀 번호 설정 (필요에 따라 변경 가능)
GPIO_PIN_RED = 16
GPIO_PIN_BLUE = 20
GPIO_PIN_GREEN = 21
DEVICE_FILE = "/dev/EMB_RGB_LED"

def write_to_device(value):
    with open(DEVICE_FILE, "w") as file:
        file.write(value)

def initialize_gpio():
    for pin in [GPIO_PIN_RED, GPIO_PIN_BLUE, GPIO_PIN_GREEN]:
        os.system(f"gpio export {pin} out")

def release_gpio():
    for pin in [GPIO_PIN_RED, GPIO_PIN_BLUE, GPIO_PIN_GREEN]:
        os.system(f"gpio unexport {pin}")

try:
    # 장치 파일 존재 여부 확인
    if not os.path.exists(DEVICE_FILE):
        print(f"오류: {DEVICE_FILE}가 존재하지 않습니다. 커널 모듈이 로드되었는지 확인하세요.")
        exit(1)

    # GPIO 핀 초기화
    #initialize_gpio()

    # 각 색상의 LED를 5번 깜박임
    for _ in range(5):
        print("빨간색 켜기")
        write_to_device("1")  # 빨간색 LED 켜기
        time.sleep(1)
        write_to_device("0")  # 빨간색 LED 끄기

        print("파란색 켜기")
        write_to_device("3")  # 파란색 LED 켜기
        time.sleep(1)
        write_to_device("2")  # 파란색 LED 끄기

        print("초록색 켜기")
        write_to_device("5")  # 초록색 LED 켜기
        time.sleep(1)
        write_to_device("4")  # 초록색 LED 끄기

finally:
    # GPIO 핀 해제
    # release_gpio()
    pass
