#include "wifi_promisc.h"
#include "json_output.h"
#include "esp_wifi.h"
#include <stdio.h>

static volatile bool s_enabled;
static volatile uint32_t s_frame_count;

static const char *pkt_type_name(wifi_promiscuous_pkt_type_t type) {
    switch (type) {
        case WIFI_PKT_MGMT: return "mgmt";
        case WIFI_PKT_CTRL: return "ctrl";
        case WIFI_PKT_DATA: return "data";
        case WIFI_PKT_MISC: return "misc";
        default: return "unknown";
    }
}

static void cb(void *buf, wifi_promiscuous_pkt_type_t type) {
    const wifi_promiscuous_pkt_t *pkt = (const wifi_promiscuous_pkt_t *)buf;
    char line[256];
    if (!s_enabled || pkt == NULL || type == WIFI_PKT_MISC) {
        return;
    }
    s_frame_count++;
    snprintf(line, sizeof(line),
             "{\"type\":\"wifi_promisc_frame\",\"frame_type\":\"%s\",\"rssi\":%d,\"channel\":%u,\"sig_len\":%u,\"timestamp\":%u,\"note\":\"partial metadata only; no decryption or full packet capture\"}",
             pkt_type_name(type), pkt->rx_ctrl.rssi, pkt->rx_ctrl.channel, pkt->rx_ctrl.sig_len, (unsigned)json_now_ms());
    json_send_line(line);
}

void wifi_promisc_init(void) { s_enabled = false; s_frame_count = 0; }

void wifi_promisc_start(void) {
    esp_wifi_set_promiscuous_rx_cb(cb);
    esp_err_t err = esp_wifi_set_promiscuous(true);
    if (err != ESP_OK) {
        json_send_error("esp_wifi_error", "Failed to enable promiscuous mode", "wifi_promisc_start");
        return;
    }
    s_enabled = true;
    json_send_line("{\"ok\":true,\"type\":\"wifi_promisc\",\"state\":\"on\",\"limitations\":[\"partial metadata only\",\"no decryption\",\"not full pcap\"]}");
}

void wifi_promisc_stop(void) {
    esp_wifi_set_promiscuous(false);
    s_enabled = false;
    json_send_line("{\"ok\":true,\"type\":\"wifi_promisc\",\"state\":\"off\"}");
}

void wifi_promisc_poll(void) {}
bool wifi_promisc_is_enabled(void) { return s_enabled; }
