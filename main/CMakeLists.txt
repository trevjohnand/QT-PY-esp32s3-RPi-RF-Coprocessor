idf_component_register(
    SRCS
        "main.c"
        "serial_console.c"
        "command_router.c"
        "json_output.c"
        "wifi_control.c"
        "wifi_scan.c"
        "wifi_promisc.c"
        "ble_scan.c"
        "event_buffer.c"
        "diagnostics.c"
        "config_store.c"
    INCLUDE_DIRS "."
    REQUIRES nvs_flash esp_wifi esp_event esp_netif bt driver json
)
