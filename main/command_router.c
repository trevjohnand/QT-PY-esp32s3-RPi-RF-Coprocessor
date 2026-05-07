#include "command_router.h"
#include "json_output.h"
#include "wifi_control.h"
#include "wifi_scan.h"
#include "wifi_promisc.h"
#include "ble_scan.h"
#include "diagnostics.h"
#include "event_buffer.h"
#include "common.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

static int parse_int_arg(const char *value, int default_value) {
    if (value == NULL || value[0] == '\0') return default_value;
    return atoi(value);
}

static bool is_true_arg(const char *value) {
    return value != NULL && (strcmp(value, "1") == 0 || strcmp(value, "true") == 0 || strcmp(value, "passive") == 0);
}

static void send_capabilities(void) {
    char line[1024];
    snprintf(line, sizeof(line),
             "{\"ok\":true,\"type\":\"capabilities\","
             "\"supported_features\":[\"wifi_radio\",\"wifi_active_scan\",\"wifi_passive_scan\",\"wifi_per_channel_scan\",\"wifi_repeated_scan\",\"wifi_channel_control\",\"wifi_mac\",\"wifi_country\",\"wifi_promiscuous_metadata\",\"ble_active_scan\",\"ble_device_list\",\"ble_mac_tracking\",\"rssi_history\",\"jsonl_events\",\"nvs_config\",\"diagnostics\",\"self_test\",\"event_flood_test\"],"
             "\"unsupported_features\":[\"wifi_frame_decryption\",\"full_pcap_capture\",\"802_11_injection\",\"classic_bluetooth\"],"
             "\"limitations\":[\"Wi-Fi promiscuous mode exposes partial frame metadata only\",\"BLE and Wi-Fi share one radio and can reduce each other throughput\",\"No hardware validation has been performed in this build environment\"],"
             "\"buffers\":{\"MAX_WIFI_RESULTS\":%d,\"MAX_BLE_RESULTS\":%d,\"MAX_TRACKED_DEVICES\":%d,\"RSSI_HISTORY_DEPTH\":%d,\"FRAME_BUFFER_SIZE\":%d,\"EVENT_QUEUE_SIZE\":%d},"
             "\"event_queue\":{\"depth\":%u,\"dropped\":%u},\"timestamp\":%u}",
             MAX_WIFI_RESULTS, MAX_BLE_RESULTS, MAX_TRACKED_DEVICES, RSSI_HISTORY_DEPTH, FRAME_BUFFER_SIZE, EVENT_QUEUE_SIZE,
             (unsigned)event_buffer_depth(), (unsigned)event_buffer_dropped(), (unsigned)json_now_ms());
    json_send_line(line);
}

void command_router_handle(const char *line) {
    char work[256];
    char *argv[6] = {0};
    int argc = 0;
    char *save = NULL;
    if (line == NULL) {
        json_send_error("bad_request", "Null command", "");
        return;
    }
    snprintf(work, sizeof(work), "%s", line);
    for (char *tok = strtok_r(work, " \t\r\n", &save); tok != NULL && argc < 6; tok = strtok_r(NULL, " \t\r\n", &save)) {
        argv[argc++] = tok;
    }
    if (argc == 0) {
        json_send_error("bad_request", "Empty command", "");
        return;
    }
    const char *cmd = argv[0];
    if (strcmp(cmd, "capabilities") == 0) send_capabilities();
    else if (strcmp(cmd, "wifi_radio_on") == 0) wifi_control_radio_on();
    else if (strcmp(cmd, "wifi_radio_off") == 0) wifi_control_radio_off();
    else if (strcmp(cmd, "wifi_scan_all") == 0) wifi_scan_start(false, 0, 120);
    else if (strcmp(cmd, "wifi_scan_passive") == 0) wifi_scan_start(true, parse_int_arg(argv[1], 0), parse_int_arg(argv[2], 120));
    else if (strcmp(cmd, "wifi_scan_channel") == 0) wifi_scan_start(false, parse_int_arg(argv[1], 0), parse_int_arg(argv[2], 120));
    else if (strcmp(cmd, "wifi_scan_results") == 0) wifi_scan_results();
    else if (strcmp(cmd, "wifi_scan_repeat_start") == 0) wifi_scan_repeated_start(is_true_arg(argv[1]), parse_int_arg(argv[2], 5000), parse_int_arg(argv[3], 0));
    else if (strcmp(cmd, "wifi_scan_repeat_stop") == 0) wifi_scan_repeated_stop();
    else if (strcmp(cmd, "wifi_set_channel") == 0) wifi_control_set_channel(parse_int_arg(argv[1], 1));
    else if (strcmp(cmd, "wifi_get_mac") == 0) wifi_control_get_mac();
    else if (strcmp(cmd, "wifi_set_country") == 0) argc >= 2 ? wifi_control_set_country(argv[1]) : json_send_error("bad_argument", "Expected country code", cmd);
    else if (strcmp(cmd, "wifi_promisc_start") == 0) wifi_promisc_start();
    else if (strcmp(cmd, "wifi_promisc_stop") == 0) wifi_promisc_stop();
    else if (strcmp(cmd, "wifi_frame_decrypt") == 0) json_send_unsupported("wifi_frame_decrypt");
    else if (strcmp(cmd, "wifi_packet_inject") == 0) json_send_unsupported("wifi_packet_inject");
    else if (strcmp(cmd, "ble_scan_start") == 0) ble_scan_start(parse_int_arg(argv[1], 0));
    else if (strcmp(cmd, "ble_scan_stop") == 0) ble_scan_stop();
    else if (strcmp(cmd, "ble_devices") == 0) ble_scan_list();
    else if (strcmp(cmd, "track_mac_start") == 0) argc >= 2 ? track_mac_start(argv[1]) : json_send_error("bad_argument", "Expected MAC address", cmd);
    else if (strcmp(cmd, "track_mac_stop") == 0) argc >= 2 ? track_mac_stop(argv[1]) : json_send_error("bad_argument", "Expected MAC address", cmd);
    else if (strcmp(cmd, "track_mac_rssi") == 0) argc >= 2 ? track_mac_rssi(argv[1]) : json_send_error("bad_argument", "Expected MAC address", cmd);
    else if (strcmp(cmd, "diagnostics") == 0) diagnostics_emit();
    else if (strcmp(cmd, "self_test") == 0) diagnostics_self_test();
    else if (strcmp(cmd, "event_flood_test") == 0) diagnostics_event_flood(parse_int_arg(argv[1], EVENT_QUEUE_SIZE * 2));
    else if (strcmp(cmd, "diagnostics_reset") == 0) diagnostics_reset_stats();
    else json_send_error("invalid_command", "Unknown command", cmd);
}
