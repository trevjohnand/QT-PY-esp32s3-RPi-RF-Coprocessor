# Commands

All commands are newline-delimited text. All responses are JSONL: one valid JSON object per line.

## Capability and diagnostics

- `capabilities`
- `diagnostics`
- `diagnostics_reset`
- `self_test`
- `event_flood_test <count>`

## Wi-Fi control

- `wifi_radio_on`
- `wifi_radio_off`
- `wifi_set_channel <1-14>`
- `wifi_get_mac`
- `wifi_set_country <CC>`

## Wi-Fi scans

- `wifi_scan_all`
- `wifi_scan_passive [channel] [duration_ms]`
- `wifi_scan_channel <channel> [duration_ms]`
- `wifi_scan_results`
- `wifi_scan_repeat_start <passive|active> <interval_ms> [channel]`
- `wifi_scan_repeat_stop`

## Wi-Fi promiscuous metadata

- `wifi_promisc_start`
- `wifi_promisc_stop`

Promiscuous output is metadata only. The firmware does not claim full pcap capture, payload decryption, or frame injection.

Explicit unsupported commands:

- `wifi_frame_decrypt`
- `wifi_packet_inject`

## BLE

- `ble_scan_start [duration_s]`
- `ble_scan_stop`
- `ble_devices`
- `track_mac_start <mac>`
- `track_mac_stop <mac>`
- `track_mac_rssi <mac>`

## Error model

Errors use this shape:

```json
{"ok":false,"error":"error_type","message":"...","command":"...","timestamp":123456}
```

Unsupported features use this shape:

```json
{"ok":false,"error":"unsupported","feature":"feature_name","message":"Not supported by ESP32-S3 or current ESP-IDF build."}
```
