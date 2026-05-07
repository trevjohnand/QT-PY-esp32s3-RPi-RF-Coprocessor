#include "serial_console.h"
#include "command_router.h"
#include "event_buffer.h"
#include "json_output.h"
#include "wifi_control.h"
#include "wifi_scan.h"
#include "wifi_promisc.h"
#include "ble_scan.h"
#include "diagnostics.h"
#include "config_store.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static void command_task(void *arg) {
    (void)arg;
    char line[256];
    while (1) {
        if (serial_console_readline(line, sizeof(line))) {
            command_router_handle(line);
        }
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

static void response_task(void *arg) {
    (void)arg;
    event_msg_t msg;
    while (1) {
        if (event_buffer_pop(&msg, pdMS_TO_TICKS(100))) {
            serial_console_write(msg.payload);
        }
    }
}

static void wifi_task(void *arg) {
    (void)arg;
    while (1) {
        wifi_scan_poll();
        wifi_promisc_poll();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void ble_task(void *arg) {
    (void)arg;
    while (1) {
        ble_scan_poll();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

static void event_dispatch_task(void *arg) {
    (void)arg;
    uint32_t last_dropped = 0;
    while (1) {
        uint32_t dropped = event_buffer_dropped();
        if (dropped != last_dropped) {
            char line[160];
            snprintf(line, sizeof(line), "{\"type\":\"event_buffer\",\"dropped_events\":%u,\"timestamp\":%u}",
                     (unsigned)dropped, (unsigned)json_now_ms());
            json_send_line(line);
            last_dropped = dropped;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void diagnostics_task(void *arg) {
    (void)arg;
    while (1) {
        diagnostics_poll();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void app_main(void) {
    serial_console_init();
    event_buffer_init();
    json_output_init();
    config_store_init();
    wifi_control_init();
    wifi_scan_init();
    wifi_promisc_init();
    ble_scan_init();
    diagnostics_init();

    xTaskCreate(command_task, "command_task", 4096, NULL, 6, NULL);
    xTaskCreate(response_task, "response_task", 4096, NULL, 5, NULL);
    xTaskCreatePinnedToCore(event_dispatch_task, "event_dispatch_task", 4096, NULL, 4, NULL, 0);
    xTaskCreatePinnedToCore(wifi_task, "wifi_task", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 4, NULL, 1);
    xTaskCreatePinnedToCore(diagnostics_task, "diagnostics_task", 4096, NULL, 2, NULL, 0);
}
