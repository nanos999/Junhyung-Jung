import os
import time

BUZZER_DEVICE = "/dev/buzzer"

def main():
    try:
        # Open the buzzer device file
        buzzer_fd = os.open(BUZZER_DEVICE, os.O_RDWR)

        # Trigger the buzzer
        print("Triggering the buzzer...")
        os.write(buzzer_fd, b'1')  # You can write different data depending on your driver's implementation

        # Wait for a moment
        time.sleep(2)

        # Stop the buzzer
        print("Stopping the buzzer...")
        os.write(buzzer_fd, b'0')  # You can write different data depending on your driver's implementation

    except Exception as e:
        print(f"Error: {e}")
    finally:
        # Close the buzzer device file
        if 'buzzer_fd' in locals():
            os.close(buzzer_fd)

if __name__ == "__main__":
    main()
