#include "ble_scan.h"
#include "json_output.h"
#include "common.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

typedef struct {
    bool used;
    bool tracked;
    esp_bd_addr_t addr;
    int rssi;
    uint32_t last_seen_ms;
    int history[RSSI_HISTORY_DEPTH];
    uint8_t history_head;
    uint8_t history_count;
    char name[32];
} ble_device_t;

static ble_device_t s_devices[MAX_BLE_RESULTS];
static bool s_ready;
static bool s_scanning;
static uint32_t s_scan_stop_ms;
static uint32_t s_tracked_count;

static void json_escape_ascii(const char *in, char *out, size_t out_len) {
    size_t j = 0;
    if (out_len == 0) return;
    for (size_t i = 0; in != NULL && in[i] != '\0' && j + 2 < out_len; i++) {
        unsigned char c = (unsigned char)in[i];
        if (c == '"' || c == '\\') {
            out[j++] = '\\';
            out[j++] = (char)c;
        } else if (c >= 32 && c <= 126) {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
}

static void addr_to_str(const uint8_t *addr, char *out, size_t out_len) {
    snprintf(out, out_len, "%02x:%02x:%02x:%02x:%02x:%02x", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);
}

static bool parse_mac(const char *mac, uint8_t out[6]) {
    unsigned int b[6];
    if (mac == NULL || sscanf(mac, "%02x:%02x:%02x:%02x:%02x:%02x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) != 6) {
        return false;
    }
    for (int i = 0; i < 6; i++) out[i] = (uint8_t)b[i];
    return true;
}

static ble_device_t *find_or_alloc(const uint8_t *addr) {
    int free_idx = -1;
    for (int i = 0; i < MAX_BLE_RESULTS; i++) {
        if (s_devices[i].used && memcmp(s_devices[i].addr, addr, 6) == 0) return &s_devices[i];
        if (!s_devices[i].used && free_idx < 0) free_idx = i;
    }
    if (free_idx < 0) return NULL;
    ble_device_t *dev = &s_devices[free_idx];
    memset(dev, 0, sizeof(*dev));
    dev->used = true;
    memcpy(dev->addr, addr, 6);
    return dev;
}

static void remember_name(ble_device_t *dev, uint8_t *adv, uint8_t adv_len) {
    uint8_t name_len = 0;
    uint8_t *name = esp_ble_resolve_adv_data(adv, ESP_BLE_AD_TYPE_NAME_CMPL, &name_len);
    if (name == NULL) name = esp_ble_resolve_adv_data(adv, ESP_BLE_AD_TYPE_NAME_SHORT, &name_len);
    if (name == NULL || name_len == 0) return;
    size_t copy = name_len < sizeof(dev->name) - 1 ? name_len : sizeof(dev->name) - 1;
    memcpy(dev->name, name, copy);
    dev->name[copy] = '\0';
}

static void push_rssi(ble_device_t *dev, int rssi) {
    dev->rssi = rssi;
    dev->last_seen_ms = json_now_ms();
    dev->history[dev->history_head] = rssi;
    dev->history_head = (dev->history_head + 1) % RSSI_HISTORY_DEPTH;
    if (dev->history_count < RSSI_HISTORY_DEPTH) dev->history_count++;
}

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (event == ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT) {
        json_send_line("{\"ok\":true,\"type\":\"ble_scan_params_set\"}");
        return;
    }
    if (event == ESP_GAP_BLE_SCAN_START_COMPLETE_EVT) {
        s_scanning = param->scan_start_cmpl.status == ESP_BT_STATUS_SUCCESS;
        if (s_scanning) {
            json_send_line("{\"ok\":true,\"type\":\"ble_scan\",\"state\":\"on\"}");
        } else {
            json_send_error("esp_ble_error", "BLE scan failed to start", "ble_scan_start");
        }
        return;
    }
    if (event == ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT) {
        s_scanning = false;
        json_send_line("{\"ok\":true,\"type\":\"ble_scan\",\"state\":\"off\"}");
        return;
    }
    if (event != ESP_GAP_BLE_SCAN_RESULT_EVT) return;
    if (param->scan_rst.search_evt != ESP_GAP_SEARCH_INQ_RES_EVT) return;

    ble_device_t *dev = find_or_alloc(param->scan_rst.bda);
    if (dev == NULL) {
        json_send_error("buffer_full", "BLE device table is full", "ble_scan_event");
        return;
    }
    remember_name(dev, param->scan_rst.ble_adv, param->scan_rst.adv_data_len);
    push_rssi(dev, param->scan_rst.rssi);

    char addr[24];
    char line[256];
    char escaped_name[64];
    addr_to_str(dev->addr, addr, sizeof(addr));
    json_escape_ascii(dev->name, escaped_name, sizeof(escaped_name));
    snprintf(line, sizeof(line), "{\"type\":\"ble_adv\",\"addr\":\"%s\",\"rssi\":%d,\"name\":\"%s\",\"tracked\":%s,\"timestamp\":%u}",
             addr, dev->rssi, escaped_name, dev->tracked ? "true" : "false", (unsigned)dev->last_seen_ms);
    json_send_line(line);
}

void ble_scan_init(void) {
    memset(s_devices, 0, sizeof(s_devices));
    s_tracked_count = 0;
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if (esp_bt_controller_init(&bt_cfg) != ESP_OK || esp_bt_controller_enable(ESP_BT_MODE_BLE) != ESP_OK ||
        esp_bluedroid_init() != ESP_OK || esp_bluedroid_enable() != ESP_OK || esp_ble_gap_register_callback(gap_cb) != ESP_OK) {
        json_send_error("esp_ble_error", "Failed to initialize BLE stack", "ble_scan_init");
        return;
    }
    esp_ble_scan_params_t scan_params = {
        .scan_type = BLE_SCAN_TYPE_ACTIVE,
        .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
        .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
        .scan_interval = 0x50,
        .scan_window = 0x30,
        .scan_duplicate = BLE_SCAN_DUPLICATE_DISABLE,
    };
    esp_ble_gap_set_scan_params(&scan_params);
    s_ready = true;
}

void ble_scan_start(int duration_s) {
    if (!s_ready) {
        json_send_error("not_ready", "BLE stack is not initialized", "ble_scan_start");
        return;
    }
    if (duration_s < 0) duration_s = 0;
    s_scan_stop_ms = duration_s == 0 ? 0 : json_now_ms() + (uint32_t)duration_s * 1000U;
    esp_err_t err = esp_ble_gap_start_scanning((uint32_t)duration_s);
    if (err != ESP_OK) json_send_error("esp_ble_error", "Failed to request BLE scan start", "ble_scan_start");
}

void ble_scan_stop(void) {
    if (!s_ready) {
        json_send_error("not_ready", "BLE stack is not initialized", "ble_scan_stop");
        return;
    }
    esp_ble_gap_stop_scanning();
}

void ble_scan_list(void) {
    char addr[24];
    char line[256];
    char escaped_name[64];
    uint32_t count = 0;
    for (int i = 0; i < MAX_BLE_RESULTS; i++) {
        if (!s_devices[i].used) continue;
        count++;
        addr_to_str(s_devices[i].addr, addr, sizeof(addr));
        json_escape_ascii(s_devices[i].name, escaped_name, sizeof(escaped_name));
        snprintf(line, sizeof(line), "{\"type\":\"ble_device\",\"addr\":\"%s\",\"rssi\":%d,\"name\":\"%s\",\"tracked\":%s,\"last_seen_ms\":%u}",
                 addr, s_devices[i].rssi, escaped_name, s_devices[i].tracked ? "true" : "false", (unsigned)s_devices[i].last_seen_ms);
        json_send_line(line);
    }
    snprintf(line, sizeof(line), "{\"ok\":true,\"type\":\"ble_devices_done\",\"count\":%u}", (unsigned)count);
    json_send_line(line);
}

void track_mac_start(const char *mac) {
    uint8_t addr[6];
    if (!parse_mac(mac, addr)) {
        json_send_error("bad_argument", "Expected MAC address aa:bb:cc:dd:ee:ff", "track_mac_start");
        return;
    }
    ble_device_t *dev = find_or_alloc(addr);
    if (dev == NULL) {
        json_send_error("buffer_full", "Device tracking table is full", "track_mac_start");
        return;
    }
    if (!dev->tracked && s_tracked_count >= MAX_TRACKED_DEVICES) {
        json_send_error("buffer_full", "Tracked MAC table is full", "track_mac_start");
        return;
    }
    if (!dev->tracked) s_tracked_count++;
    dev->tracked = true;
    json_send_line("{\"ok\":true,\"type\":\"track_mac\",\"state\":\"on\"}");
}

void track_mac_stop(const char *mac) {
    uint8_t addr[6];
    if (!parse_mac(mac, addr)) {
        json_send_error("bad_argument", "Expected MAC address aa:bb:cc:dd:ee:ff", "track_mac_stop");
        return;
    }
    for (int i = 0; i < MAX_BLE_RESULTS; i++) {
        if (s_devices[i].used && memcmp(s_devices[i].addr, addr, 6) == 0 && s_devices[i].tracked) {
            s_devices[i].tracked = false;
            if (s_tracked_count > 0) s_tracked_count--;
        }
    }
    json_send_line("{\"ok\":true,\"type\":\"track_mac\",\"state\":\"off\"}");
}

void track_mac_rssi(const char *mac) {
    uint8_t addr[6];
    char line[128];
    if (!parse_mac(mac, addr)) {
        json_send_error("bad_argument", "Expected MAC address aa:bb:cc:dd:ee:ff", "track_mac_rssi");
        return;
    }
    for (int i = 0; i < MAX_BLE_RESULTS; i++) {
        if (!s_devices[i].used || memcmp(s_devices[i].addr, addr, 6) != 0) continue;
        json_send_line("{\"type\":\"rssi_history_start\"}");
        for (int n = 0; n < s_devices[i].history_count; n++) {
            int idx = (s_devices[i].history_head + RSSI_HISTORY_DEPTH - s_devices[i].history_count + n) % RSSI_HISTORY_DEPTH;
            snprintf(line, sizeof(line), "{\"type\":\"rssi_sample\",\"rssi\":%d}", s_devices[i].history[idx]);
            json_send_line(line);
        }
        json_send_line("{\"ok\":true,\"type\":\"rssi_history_done\"}");
        return;
    }
    json_send_error("not_found", "MAC address is not tracked or observed", "track_mac_rssi");
}

void ble_scan_poll(void) {
    if (s_scanning && s_scan_stop_ms != 0 && json_now_ms() >= s_scan_stop_ms) {
        ble_scan_stop();
        s_scan_stop_ms = 0;
    }
}
