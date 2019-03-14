import sys
import time
import serial


def main():
    com_port = 'COM6'
    send_delay = 0.50
    try:
        serial_port = serial.Serial(com_port, baudrate=115200, timeout=0.010)
    except serial.SerialException as e:
        print(e)
        sys.exit(1)

    try:
        while True:
            print("send")
            serial_port.write(b'@CEREA;5.0;-1;1;1;1;1;1;1;1;1;1;1;END\r\n')
            time.sleep(send_delay)
            print(serial_port.read_until())
            serial_port.write(b'@HIDRAU;1;END\r\n')
            time.sleep(send_delay)
            print(serial_port.read_until())
    except KeyboardInterrupt:
        print('Keyboard interrupt by user - exit')
        sys.exit(1)


if __name__ == '__main__':
    main()
