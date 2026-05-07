#include "json_output.h"
#include "event_buffer.h"
#include "esp_timer.h"
#include <stdio.h>
#include <string.h>

void json_output_init(void) {}

uint32_t json_now_ms(void) { return (uint32_t)(esp_timer_get_time() / 1000ULL); }

void json_send_line(const char *line) { event_buffer_push(line); }

void json_send_error(const char *error, const char *message, const char *command) {
    char buf[512];
    snprintf(buf, sizeof(buf), "{\"ok\":false,\"error\":\"%s\",\"message\":\"%s\",\"command\":\"%s\",\"timestamp\":%u}",
             error, message, command ? command : "", json_now_ms());
    json_send_line(buf);
}

void json_send_unsupported(const char *feature) {
    char buf[512];
    snprintf(buf, sizeof(buf), "{\"ok\":false,\"error\":\"unsupported\",\"feature\":\"%s\",\"message\":\"Not supported by ESP32-S3 or current ESP-IDF build.\"}",
             feature);
    json_send_line(buf);
}
