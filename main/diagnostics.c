#include "diagnostics.h"
#include "json_output.h"
#include "event_buffer.h"
#include "esp_heap_caps.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>

static uint32_t s_boot_ms;

void diagnostics_init(void) { s_boot_ms = json_now_ms(); }

void diagnostics_emit(void) {
    char line[384];
    snprintf(line, sizeof(line),
             "{\"ok\":true,\"type\":\"diagnostics\",\"uptime_ms\":%u,\"free_heap\":%u,\"minimum_free_heap\":%u,\"event_queue_depth\":%u,\"dropped_events\":%u,\"task_count\":%u,\"timestamp\":%u}",
             (unsigned)(json_now_ms() - s_boot_ms),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_8BIT),
             (unsigned)heap_caps_get_minimum_free_size(MALLOC_CAP_8BIT),
             (unsigned)event_buffer_depth(),
             (unsigned)event_buffer_dropped(),
             (unsigned)uxTaskGetNumberOfTasks(),
             (unsigned)json_now_ms());
    json_send_line(line);
}

void diagnostics_self_test(void) {
    bool ok = event_buffer_push("{\"type\":\"self_test_event_path\"}");
    char line[256];
    snprintf(line, sizeof(line), "{\"ok\":%s,\"type\":\"self_test\",\"event_path\":%s,\"heap_nonzero\":%s,\"timestamp\":%u}",
             ok ? "true" : "false", ok ? "true" : "false", heap_caps_get_free_size(MALLOC_CAP_8BIT) > 0 ? "true" : "false", (unsigned)json_now_ms());
    json_send_line(line);
}

void diagnostics_event_flood(int count) {
    if (count < 0) count = 0;
    if (count > EVENT_QUEUE_SIZE * 8) count = EVENT_QUEUE_SIZE * 8;
    uint32_t before = event_buffer_dropped();
    char line[160];
    for (int i = 0; i < count; i++) {
        snprintf(line, sizeof(line), "{\"type\":\"event_flood\",\"seq\":%d,\"timestamp\":%u}", i, (unsigned)json_now_ms());
        event_buffer_push(line);
    }
    snprintf(line, sizeof(line), "{\"ok\":true,\"type\":\"event_flood_done\",\"requested\":%d,\"dropped_delta\":%u}", count, (unsigned)(event_buffer_dropped() - before));
    json_send_line(line);
}

void diagnostics_reset_stats(void) {
    event_buffer_reset_stats();
    json_send_line("{\"ok\":true,\"type\":\"diagnostics_reset\"}");
}

void diagnostics_poll(void) {}
