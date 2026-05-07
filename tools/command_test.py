#!/usr/bin/env python3
import argparse
import json
import serial
import sys

parser = argparse.ArgumentParser(description='Exercise RF coprocessor JSONL command surface.')
parser.add_argument('port', nargs='?', default='/dev/ttyACM0')
parser.add_argument('--loops', type=int, default=1)
parser.add_argument('--timeout', type=float, default=2.0)
args = parser.parse_args()

cmds = [
    'capabilities',
    'wifi_get_mac',
    'diagnostics',
    'self_test',
    'wifi_frame_decrypt',
]

ser = serial.Serial(args.port, 115200, timeout=args.timeout)
for loop in range(args.loops):
    for cmd in cmds:
        ser.write((cmd + '\n').encode())
        line = ser.readline().decode(errors='ignore').strip()
        if not line:
            print(f'timeout waiting for {cmd}', file=sys.stderr)
            sys.exit(1)
        json.loads(line)
        print(f'{loop}:{cmd}: {line}')
