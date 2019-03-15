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

    print('Send commands with', send_delay, 'seconds delay')

    i = 0
    command_strings =  ['@CEREA;5.0;-1;1;END',
                        '@CEREA;5.0;-1;0;END',
                        '@CEREA;5.0;-1;1;1;1;1;1;1;1;1;1;1;END',
                        '@HIDRAU;1;END',
                        '@STATUSAM;1;1;END']

    try:
        while True:
            command = command_strings[i % len(command_strings)]
            print('>', command)
            serial_port.write((command + '\r\n').encode())
            time.sleep(send_delay)
            print(serial_port.read_until())
            i = i + 1
    except KeyboardInterrupt:
        print('Keyboard interrupt by user - exit')
        sys.exit(1)


if __name__ == '__main__':
    cerea_simulation()
