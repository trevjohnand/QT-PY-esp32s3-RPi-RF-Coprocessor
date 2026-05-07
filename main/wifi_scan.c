#include "wifi_scan.h"
#include "wifi_promisc.h"
#include "json_output.h"
#include "common.h"
#include "esp_wifi.h"
#include <stdio.h>
#include <string.h>

static wifi_ap_record_t s_results[MAX_WIFI_RESULTS];
static uint16_t s_count;
static bool s_repeated;
static bool s_repeat_passive;
static int s_repeat_interval_ms;
static int s_repeat_channel;
static uint32_t s_last_scan_ms;

static void json_escape_ssid(const uint8_t *ssid, char *out, size_t out_len) {
    size_t j = 0;
    for (size_t i = 0; i < 32 && ssid[i] != 0 && j + 2 < out_len; i++) {
        unsigned char c = ssid[i];
        if (c == '"' || c == '\\') {
            if (j + 2 >= out_len) break;
            out[j++] = '\\';
            out[j++] = (char)c;
        } else if (c >= 32 && c <= 126) {
            out[j++] = (char)c;
        }
    }
    out[j] = '\0';
}

void wifi_scan_init(void) {
    s_count = 0;
    s_repeated = false;
}

void wifi_scan_start(bool passive, int channel, int duration_ms) {
    if (wifi_promisc_is_enabled() && !passive) {
        json_send_error("conflict", "Active scanning is blocked while promiscuous mode is enabled", "wifi_scan_start");
        return;
    }
    wifi_scan_config_t cfg = {0};
    cfg.channel = (uint8_t)channel;
    cfg.scan_type = passive ? WIFI_SCAN_TYPE_PASSIVE : WIFI_SCAN_TYPE_ACTIVE;
    cfg.scan_time.passive = duration_ms > 0 ? (uint32_t)duration_ms : 120;
    esp_err_t err = esp_wifi_scan_start(&cfg, false);
    if (err != ESP_OK) {
        json_send_error("esp_wifi_error", "Failed to start Wi-Fi scan", "wifi_scan_start");
        return;
    }
    json_send_line("{\"ok\":true,\"type\":\"wifi_scan_started\"}");
}

void wifi_scan_results(void) {
    char line[384];
    char ssid[96];
    for (int i = 0; i < s_count; i++) {
        json_escape_ssid(s_results[i].ssid, ssid, sizeof(ssid));
        snprintf(line, sizeof(line),
                 "{\"type\":\"wifi_ap\",\"ssid\":\"%s\",\"bssid\":\"%02x:%02x:%02x:%02x:%02x:%02x\",\"channel\":%u,\"rssi\":%d,\"authmode\":%d,\"timestamp\":%u}",
                 ssid, s_results[i].bssid[0], s_results[i].bssid[1], s_results[i].bssid[2], s_results[i].bssid[3], s_results[i].bssid[4], s_results[i].bssid[5],
                 s_results[i].primary, s_results[i].rssi, s_results[i].authmode, (unsigned)json_now_ms());
        json_send_line(line);
    }
    snprintf(line, sizeof(line), "{\"ok\":true,\"type\":\"wifi_scan_results_done\",\"count\":%u}", s_count);
    json_send_line(line);
}

void wifi_scan_repeated_start(bool passive, int interval_ms, int channel) {
    s_repeated = true;
    s_repeat_passive = passive;
    s_repeat_interval_ms = interval_ms < 1000 ? 1000 : interval_ms;
    s_repeat_channel = channel;
    s_last_scan_ms = 0;
    json_send_line("{\"ok\":true,\"type\":\"wifi_scan_repeated\",\"state\":\"on\"}");
}

void wifi_scan_repeated_stop(void) {
    s_repeated = false;
    json_send_line("{\"ok\":true,\"type\":\"wifi_scan_repeated\",\"state\":\"off\"}");
}

void wifi_scan_poll(void) {
    uint32_t now = json_now_ms();
    uint16_t count = MAX_WIFI_RESULTS;
    if (esp_wifi_scan_get_ap_records(&count, s_results) == ESP_OK && count > 0) {
        s_count = count;
        char line[128];
        snprintf(line, sizeof(line), "{\"ok\":true,\"type\":\"wifi_scan_done\",\"count\":%u,\"timestamp\":%u}", s_count, (unsigned)now);
        json_send_line(line);
    }
    if (s_repeated && (s_last_scan_ms == 0 || now - s_last_scan_ms >= (uint32_t)s_repeat_interval_ms)) {
        s_last_scan_ms = now;
        wifi_scan_start(s_repeat_passive, s_repeat_channel, 120);
    }
}
