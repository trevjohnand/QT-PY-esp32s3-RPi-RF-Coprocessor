# Build Results

This repository contains a complete ESP-IDF firmware source tree for Adafruit QT Py ESP32-S3.

## Current environment result

- Build status: **not built in this container** because `idf.py` is not installed.
- Command attempted: `idf.py --version`
- Result: `/bin/bash: line 1: idf.py: command not found`

## Expected output binaries after a successful ESP-IDF build

- `build/bootloader/bootloader.bin`
- `build/partition_table/partition-table.bin`
- `build/rf_coprocessor.bin`

Copy these into `dist/` as:

- `dist/bootloader.bin`
- `dist/partitions.bin`
- `dist/firmware.bin`
