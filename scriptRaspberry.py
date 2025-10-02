#!/usr/bin/env python3

import serial,time

if __name__ == '__main__':
    print('Running.')
    with serial.Serial("/dev/ttyACM0",9600,timeout=1) as arduino:
        time.sleep(0.1)
        if arduino.isOpen():
            print("{} connected!".format(arduino.port))
            try:
                while True:
                    cmd=input("Enter command:")
                    arduino.write(cmd.encode())
                    while arduino.inWaiting()==0: pass
                    if arduino.inWaiting()>0:
                        answer=arduino.readLine()
                        print(answer)
                        arduino.flushInput()
            except KeyboardInterrupt:
                print("Stopped")
