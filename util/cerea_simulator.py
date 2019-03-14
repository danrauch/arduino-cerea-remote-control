import sys
import time
import serial

def main():
    com_port = 'COM6'

    try:
        serial_port = serial.Serial(com_port, timeout=0.1)        
    except serial.SerialException as e:
        print(e)
        sys.exit(1)
    
    try:
        while True:    
            serial_port.write("@CEREA;5.0;-1;1;1;1;1;1;1;1;1;1;1;END")
            time.sleep(0.5)
            serial_port.write("@HIDRAU;1;END")
            print(serial_port.read_until())
            time.sleep(0.5)            
    except KeyboardInterrupt:
        print('Keyboard interrupt by user - exit')
        sys.exit(1) 


if __name__ == '__main__':
    main()