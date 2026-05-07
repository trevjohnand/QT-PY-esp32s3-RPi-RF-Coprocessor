#!/usr/bin/env python3
from pathlib import Path
import re
import sys

root = Path(__file__).resolve().parents[1]
required_files = [
    'CMakeLists.txt', 'sdkconfig.defaults', 'main/main.c', 'main/command_router.c',
    'main/event_buffer.c', 'main/wifi_scan.c', 'main/wifi_promisc.c', 'main/ble_scan.c',
    'main/diagnostics.c', 'COMMANDS.md', 'VALIDATION.md'
]
missing = [f for f in required_files if not (root / f).exists()]
if missing:
    print('missing files:', ', '.join(missing))
    sys.exit(1)

common = (root / 'main/common.h').read_text()
limits = {
    'MAX_WIFI_RESULTS': '256',
    'MAX_BLE_RESULTS': '256',
    'MAX_TRACKED_DEVICES': '128',
    'RSSI_HISTORY_DEPTH': '32',
    'FRAME_BUFFER_SIZE': '4096',
    'EVENT_QUEUE_SIZE': '1024',
}
for name, value in limits.items():
    if f'#define {name} {value}' not in common:
        print(f'bad limit {name}')
        sys.exit(1)

sdk = (root / 'sdkconfig.defaults').read_text()
for symbol in ['CONFIG_ESP_CONSOLE_USB_CDC=y', 'CONFIG_TINYUSB_CDC_ENABLED=y', 'CONFIG_ESP_CONSOLE_UART_NONE=y']:
    if symbol not in sdk:
        print(f'missing {symbol}')
        sys.exit(1)

router = (root / 'main/command_router.c').read_text()
commands = set(re.findall(r'strcmp\(cmd, "([^"]+)"\)', router))
doc = (root / 'COMMANDS.md').read_text()
undocumented = sorted(c for c in commands if c not in doc)
if undocumented:
    print('undocumented commands:', ', '.join(undocumented))
    sys.exit(1)

if 'json_send_unsupported("wifi_frame_decrypt")' not in router or 'json_send_unsupported("wifi_packet_inject")' not in router:
    print('unsupported feature responses missing')
    sys.exit(1)

print(f'source validation passed ({len(commands)} commands documented)')
