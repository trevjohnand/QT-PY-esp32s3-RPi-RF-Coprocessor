# RF Validation Plan

This firmware is structured for real RF validation, but hardware success must only be claimed after running tests on an Adafruit QT Py ESP32-S3 connected to the Raspberry Pi host.

## Required validation matrix

1. **USB CDC reliability**
   - Run `python3 tools/command_test.py /dev/ttyACM0 --loops 1000`.
   - Confirm every line is valid JSON and no command stalls.

2. **Event flood and buffer pressure**
   - Send `event_flood_test 8192`.
   - Confirm `diagnostics` reports bounded queue depth and increasing `dropped_events` instead of heap exhaustion.

3. **Wi-Fi scanning**
   - Run `wifi_radio_on`, `wifi_scan_all`, then `wifi_scan_results`.
   - Repeat with `wifi_scan_passive <channel> <duration_ms>` and `wifi_scan_repeat_start passive 5000 <channel>`.

4. **Promiscuous metadata**
   - Run `wifi_promisc_start` and observe `wifi_promisc_frame` metadata events.
   - Confirm the firmware never claims decryption, packet injection, or full pcap capture.

5. **BLE scanning and tracking**
   - Run `ble_scan_start 30`, `ble_devices`, `track_mac_start <mac>`, and `track_mac_rssi <mac>`.
   - Confirm RSSI history never exceeds `RSSI_HISTORY_DEPTH`.

6. **Wi-Fi/BLE contention**
   - Run BLE scan while repeated Wi-Fi scan is enabled.
   - Monitor `diagnostics` for dropped events, minimum free heap, and USB command responsiveness.

7. **Long-run stability**
   - Run repeated scan + BLE scan + diagnostics polling for 8+ hours.
   - Record minimum free heap, dropped event count, resets, and USB disconnects.

## Pass criteria

- No crash, watchdog reset, or heap exhaustion.
- All output lines remain valid JSON objects.
- Queue pressure causes bounded drops, not unbounded allocation.
- Unsupported features return explicit `unsupported` JSON.
