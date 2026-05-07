#pragma once
#include <stdint.h>

#define MAX_WIFI_RESULTS 256
#define MAX_BLE_RESULTS 256
#define MAX_TRACKED_DEVICES 128
#define RSSI_HISTORY_DEPTH 32
#define FRAME_BUFFER_SIZE 4096
#define EVENT_QUEUE_SIZE 1024

typedef struct {
    uint32_t timestamp_ms;
    char payload[FRAME_BUFFER_SIZE];
} event_msg_t;
