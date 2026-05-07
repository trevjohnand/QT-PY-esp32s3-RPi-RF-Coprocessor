#pragma once

void ble_scan_init(void);
void ble_scan_start(int duration_s);
void ble_scan_stop(void);
void ble_scan_list(void);
void track_mac_start(const char *mac);
void track_mac_stop(const char *mac);
void track_mac_rssi(const char *mac);
void ble_scan_poll(void);
