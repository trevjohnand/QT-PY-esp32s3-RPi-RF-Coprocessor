# QT Py ESP32-S3 RF Coprocessor Firmware (ESP-IDF)

USB-serial controlled RF telemetry coprocessor firmware for the Adafruit QT Py ESP32-S3, intended for Raspberry Pi Zero 2 W host control.

The Pi remains the application brain and keeps its own Wi-Fi. The ESP32-S3 exposes Wi-Fi and BLE control/telemetry over native USB CDC serial using newline-delimited commands and JSONL responses.

## Design principles

- ESP-IDF only: no Arduino and no CircuitPython.
- JSONL only: every output line is a valid JSON object.
- Bounded memory: fixed result tables, fixed RSSI history, fixed event queue, and dropped-event counters.
- Honest capability reporting: impossible or unsupported RF functions return explicit `unsupported` JSON instead of fake success.
- Validation-first: RF success is not claimed until hardware tests in `VALIDATION.md` pass.

## Implemented control surface

- Wi-Fi radio on/off, channel, country, MAC, active/passive/per-channel/repeated scans.
- Wi-Fi promiscuous metadata using ESP-IDF APIs with explicit limitation notes: partial metadata only, no decryption, no full pcap claim.
- BLE active scanning using ESP-IDF BLE GAP APIs, bounded device list, MAC tracking, and RSSI history.
- Diagnostics, self-test, event flood testing, queue depth, drop counters, heap metrics, and task count.
- NVS hooks for persistent radio configuration.

## Quick start

```bash
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyACM0 flash monitor
```

## Host smoke tests

```bash
python3 tools/serial_test.py /dev/ttyACM0
python3 tools/command_test.py /dev/ttyACM0 --loops 100
```

See `COMMANDS.md`, `FLASHING.md`, and `VALIDATION.md` for the full command and validation plan.
