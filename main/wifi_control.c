#include "wifi_control.h"
#include "json_output.h"
#include "config_store.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

static bool s_wifi_ready;
static bool s_wifi_started;

static void send_esp_error(const char *cmd, const char *message) {
    json_send_error("esp_wifi_error", message, cmd);
}

void wifi_control_init(void) {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if (esp_wifi_init(&cfg) != ESP_OK || esp_wifi_set_mode(WIFI_MODE_STA) != ESP_OK) {
        send_esp_error("wifi_control_init", "Failed to initialize Wi-Fi STA mode");
        return;
    }
    s_wifi_ready = true;
}

void wifi_control_radio_on(void) {
    if (!s_wifi_ready) {
        send_esp_error("wifi_radio_on", "Wi-Fi stack is not ready");
        return;
    }
    esp_err_t err = esp_wifi_start();
    if (err != ESP_OK) {
        send_esp_error("wifi_radio_on", "Failed to start Wi-Fi radio");
        return;
    }
    s_wifi_started = true;
    json_send_line("{\"ok\":true,\"type\":\"wifi_radio\",\"state\":\"on\"}");
}

void wifi_control_radio_off(void) {
    esp_err_t err = esp_wifi_stop();
    if (err != ESP_OK) {
        send_esp_error("wifi_radio_off", "Failed to stop Wi-Fi radio");
        return;
    }
    s_wifi_started = false;
    json_send_line("{\"ok\":true,\"type\":\"wifi_radio\",\"state\":\"off\"}");
}

void wifi_control_set_channel(int channel) {
    if (channel < 1 || channel > 14) {
        json_send_error("bad_argument", "Wi-Fi channel must be in range 1-14", "wifi_set_channel");
        return;
    }
    esp_err_t err = esp_wifi_set_channel((uint8_t)channel, WIFI_SECOND_CHAN_NONE);
    if (err != ESP_OK) {
        send_esp_error("wifi_set_channel", "Failed to set Wi-Fi channel");
        return;
    }
    config_store_set_i32("channel", channel);
    char line[128];
    snprintf(line, sizeof(line), "{\"ok\":true,\"type\":\"wifi_channel_set\",\"channel\":%d}", channel);
    json_send_line(line);
}

void wifi_control_get_mac(void) {
    uint8_t mac[6];
    if (esp_wifi_get_mac(WIFI_IF_STA, mac) != ESP_OK) {
        send_esp_error("wifi_get_mac", "Failed to read Wi-Fi MAC address");
        return;
    }
    char b[160];
    snprintf(b, sizeof(b), "{\"ok\":true,\"type\":\"wifi_mac\",\"mac\":\"%02x:%02x:%02x:%02x:%02x:%02x\"}",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    json_send_line(b);
}

void wifi_control_set_country(const char *cc) {
    if (cc == NULL || strlen(cc) != 2) {
        json_send_error("bad_argument", "Country code must be two characters", "wifi_set_country");
        return;
    }
    wifi_country_t c = {0};
    snprintf(c.cc, sizeof(c.cc), "%s", cc);
    c.schan = 1;
    c.nchan = 13;
    c.policy = WIFI_COUNTRY_POLICY_MANUAL;
    esp_err_t err = esp_wifi_set_country(&c);
    if (err != ESP_OK) {
        send_esp_error("wifi_set_country", "Failed to set Wi-Fi country");
        return;
    }
    json_send_line("{\"ok\":true,\"type\":\"wifi_country_set\"}");
}
