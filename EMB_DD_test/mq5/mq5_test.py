import os
import time

MQ5_DEVICE = "/dev/mq5_driver"

def read_gas_sensor_value():
    try:
        # Open the MQ-5 device file for reading
        mq5_fd = os.open(MQ5_DEVICE, os.O_RDONLY)

        # Read the gas sensor value from the device file
        gas_value_str = os.read(mq5_fd, 4).decode().strip()

        # Convert the string value to an integer
        gas_value = int(gas_value_str)

        return gas_value

    except Exception as e:
        print(f"Error reading gas sensor value: {e}")
        return None

    finally:
        # Close the MQ-5 device file
        if 'mq5_fd' in locals():
            os.close(mq5_fd)

def main():
    try:
        while True:
            # Read value of MQ-5 sensor
            gas_value = read_gas_sensor_value()

            if gas_value is not None:
                # Gas detection logic
                if gas_value == 1:
                    print("Gas detected! Warning!")
                else:
                    print("Gas not detected")
                    print(gas_value)

            # Wait for a moment
            time.sleep(1)

    except KeyboardInterrupt:
        pass

if __name__ == "__main__":
    main()


