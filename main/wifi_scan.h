#pragma once
#include <stdbool.h>

void wifi_scan_init(void);
void wifi_scan_start(bool passive, int channel, int duration_ms);
void wifi_scan_results(void);
void wifi_scan_repeated_start(bool passive, int interval_ms, int channel);
void wifi_scan_repeated_stop(void);
void wifi_scan_poll(void);
