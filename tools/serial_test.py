#!/usr/bin/env python3
import json
import serial
import sys

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyACM0'
ser = serial.Serial(port, 115200, timeout=1)
ser.write(b'capabilities\n')
line = ser.readline().decode(errors='ignore').strip()
print(line)
json.loads(line)
