# Flashing

## Build
```bash
idf.py set-target esp32s3
idf.py build
```

## Flash from Linux/Raspberry Pi
```bash
idf.py -p /dev/ttyACM0 flash monitor
```

## Flash with esptool directly
```bash
python -m esptool --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write_flash 0x0 build/bootloader/bootloader.bin 0x8000 build/partition_table/partition-table.bin 0x10000 build/rf_coprocessor.bin
```
