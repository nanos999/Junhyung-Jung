import os
import argparse
import sys
import time
import smbus
import I2C_LCD_driver

import cv2
from tflite_support.task import core
from tflite_support.task import processor
from tflite_support.task import vision
import utils

# 디바이스 파일 경로 설정
BUZZER_DEVICE = "/dev/buzzer"
MQ5_DEVICE = "/dev/mq5_driver"
LED_DEVICE = "/dev/EMB_RGB_LED"

# LED 핀 번호
GPIO_PIN_RED = 16  # For red LED
GPIO_PIN_BLUE = 20  # For blue LED
GPIO_PIN_GREEN = 21  # For green LED

# I2C 설정
bus = smbus.SMBus(1)  # I2C 버스 1 사용
AIN0 = 0x48  # 가스 센서의 I2C 주소

# LCD 설정
lcd = I2C_LCD_driver.lcd()

# LED 디바이스 파일에 값 쓰기
def write_to_led_device(value):
    with open(LED_DEVICE, "w") as file:
        file.write(value)
        
        
# 화재 감지 경계값 설정
detect_threshold = 0.6

# 가스 센서 값 읽기
def read_gas_sensor_value():
    try:
        mq5_fd = os.open(MQ5_DEVICE, os.O_RDONLY)
        gas_value_str = os.read(mq5_fd, 4).decode().strip()
        gas_value = int(gas_value_str)

        return gas_value

    except Exception as e:
        print(f"Error reading gas sensor value: {e}")
        return None

    finally:
        if 'mq5_fd' in locals():
            os.close(mq5_fd)

# 부저 켜기
def buzzer_on():
    try:
        buzzer_fd = os.open(BUZZER_DEVICE, os.O_RDWR)
        os.write(buzzer_fd, b'1')
    except Exception as e:
        print(f"Error triggering buzzer: {e}")
    finally:
        if 'buzzer_fd' in locals():
            os.close(buzzer_fd)

# 부저 끄기
def buzzer_off():
    try:
        buzzer_fd = os.open(BUZZER_DEVICE, os.O_RDWR)
        os.write(buzzer_fd, b'0')
    except Exception as e:
        print(f"Error triggering buzzer: {e}")
    finally:
        if 'buzzer_fd' in locals():
            os.close(buzzer_fd)

def run(model: str, camera_id: int, width: int, height: int, num_threads: int,
        enable_edgetpu: bool) -> None:
  # FPS 계산을 위한 변수
  counter, fps = 0, 0
  start_time = time.time()

  # 카메라에서 비디오 입력 캡처 시작
  cap = cv2.VideoCapture(camera_id)
  cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
  cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

  # 시각화 파라미터 설정
  row_size = 20  # pixels
  left_margin = 24  # pixels
  text_color = (0, 0, 255)  # red
  font_size = 1
  font_thickness = 1
  fps_avg_frame_count = 10

  # 객체 감지 모델 초기화
  base_options = core.BaseOptions(
      file_name=model, use_coral=enable_edgetpu, num_threads=num_threads)
  detection_options = processor.DetectionOptions(
      max_results=3, score_threshold=detect_threshold)
  options = vision.ObjectDetectorOptions(
      base_options=base_options, detection_options=detection_options)
  detector = vision.ObjectDetector.create_from_options(options)

  # 카메라에서 이미지를 지속적으로 캡처하고 추론 실행
  while cap.isOpened():
    success, image = cap.read()
    if not success:
      sys.exit(
          'ERROR: Unable to read from webcam. Please verify your webcam settings.'
      )

    # 가스 센서 값 읽기 (아날로그, 디지털)
    gas_value_A = bus.read_byte(AIN0)
    print("Gas value: {}".format(gas_value_A))
    gas_value_D = read_gas_sensor_value()
    
    counter += 1
    image = cv2.flip(image, 1)

    # TFLite 모델에 필요한 RGB로 이미지 변환
    rgb_image = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # RGB 이미지로부터 TensorImage 객체 생성
    input_tensor = vision.TensorImage.create_from_array(rgb_image)

    # 모델을 사용한 객체 감지 추정 실행
    detection_result = detector.detect(input_tensor)

    # 화재 감지 확인
    fire_detected = any(d.categories[0].category_name == 'tfire' and 
                        d.categories[0].score >= detect_threshold 
                        for d in detection_result.detections)

    # LCD에 현재 시간 표시
    current_date = time.strftime("%m/%d")
    current_time = time.strftime("%H:%M:%S")
    lcd.lcd_display_string(f"{current_date} {current_time}", 1)

    # 감지에 따른 LED 제어 및 메시지 표시
    if fire_detected and gas_value_D == 1:
        # 카메라와 가스 센서에 의해 화재 감지: 빨간색 및 파란색 LED 켜기, 메시지 표시
        write_to_led_device("1")  # Red on
        write_to_led_device("3")  # Blue on
        write_to_led_device("4")  # Green off
        lcd.lcd_display_string("DETECT FIRE&GAS", 2)
        buzzer_on()
    elif fire_detected:
        # 카메라에 의해 화재 감지: 빨간색 LED 켜기, 메시지 표시
        write_to_led_device("1")  # Red on
        write_to_led_device("2")  # Blue off
        write_to_led_device("4")  # Green off
        lcd.lcd_display_string("FIRE DETECTED         ", 2)
        buzzer_off()
    elif gas_value_D == 1:
        # MQ5에 의해 가스 감지: 파란색 LED 켜기, 메시지 표시
        write_to_led_device("0")  # Red off
        write_to_led_device("3")  # Blue on
        write_to_led_device("4")  # Green off
        lcd.lcd_display_string(f"GAS {gas_value_A}           ", 2)
        buzzer_off()  
    else:
        # 감지 없음
        write_to_led_device("0")  # Red off
        write_to_led_device("2")  # Blue off
        write_to_led_device("5")  # Green on
        lcd.lcd_display_string("SAFE             ", 2)
        buzzer_off()   

    # 입력 이미지에 키포인트 및 에지 그리기
    image = utils.visualize(image, detection_result)

    # FPS 계산
    if counter % fps_avg_frame_count == 0:
      end_time = time.time()
      fps = fps_avg_frame_count / (end_time - start_time)
      start_time = time.time()

    # FPS 표시
    fps_text = 'FPS = {:.1f}'.format(fps)
    text_location = (left_margin, row_size)
    cv2.putText(image, fps_text, text_location, cv2.FONT_HERSHEY_PLAIN,
                font_size, text_color, font_thickness)

    # ESC 키가 눌리면 프로그램 정지
    if cv2.waitKey(1) == 27:
      break
    cv2.imshow('object_detector', image)

    # lcd.lcd_clear()

  cap.release()
  cv2.destroyAllWindows()


def main():
  parser = argparse.ArgumentParser(
      formatter_class=argparse.ArgumentDefaultsHelpFormatter)
  parser.add_argument(
      '--model',
      help='Path of the object detection model.',
      required=False,
      default='best.tflite')
  parser.add_argument(
      '--cameraId', help='Id of camera.', required=False, type=int, default=0)
  parser.add_argument(
      '--frameWidth',
      help='Width of frame to capture from camera.',
      required=False,
      type=int,
      default=640)
  parser.add_argument(
      '--frameHeight',
      help='Height of frame to capture from camera.',
      required=False,
      type=int,
      default=480)
  parser.add_argument(
      '--numThreads',
      help='Number of CPU threads to run the model.',
      required=False,
      type=int,
      default=4)
  parser.add_argument(
      '--enableEdgeTPU',
      help='Whether to run the model on EdgeTPU.',
      action='store_true',
      required=False,
      default=False)
  args = parser.parse_args()

  run(args.model, int(args.cameraId), args.frameWidth, args.frameHeight,
      int(args.numThreads), bool(args.enableEdgeTPU))


if __name__ == '__main__':
  main()
