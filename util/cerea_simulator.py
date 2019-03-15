import sys
import time
import serial


def cerea_simulation():
    ''' 
    cerea_simulation() mimics the serial output behaviour of
    the Cerea auto-steer system. Can be used for testing and
    prototyping of e.g. Arduino controlles without the necessity
    of a working Cerea setup.

    The settings of the serial partner have to match.
    '''

    com_port = 'COM6'
    # com_port = '/dev/cu.usbmodemFA1301'
    send_delay = 0.5

    try:
        # defaults to a baudrate of 9600; no parity; 1 stop bit
        serial_port = serial.Serial(com_port, timeout=0.010)
    except serial.SerialException as e:
        print(e)
        sys.exit(1)

    print("Send commands with", send_delay, "seconds delay")
    try:
        while True:
            print("> @CEREA;5.0;-1;1;1;1;1;1;1;1;1;1;1;END")
            serial_port.write(b'@CEREA;5.0;-1;1;1;1;1;1;1;1;1;1;1;END\r\n')
            time.sleep(send_delay)
            print(serial_port.read_until())
            print("> @HIDRAU;1;END")
            serial_port.write(b'@HIDRAU;1;END\r\n')
            time.sleep(send_delay)
            print(serial_port.read_until())
    except KeyboardInterrupt:
        print('Keyboard interrupt by user - exit')
        sys.exit(1)


if __name__ == '__main__':
    cerea_simulation()
