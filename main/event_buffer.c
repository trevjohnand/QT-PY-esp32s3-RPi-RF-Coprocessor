#include "event_buffer.h"
#include "freertos/queue.h"
#include <string.h>

static QueueHandle_t s_q;
static uint32_t s_dropped;

void event_buffer_init(void) {
    s_q = xQueueCreate(EVENT_QUEUE_SIZE, sizeof(event_msg_t));
    s_dropped = 0;
}

bool event_buffer_push(const char *json_line) {
    event_msg_t m = {0};
    if (s_q == NULL || json_line == NULL) {
        s_dropped++;
        return false;
    }
    m.timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    strncpy(m.payload, json_line, sizeof(m.payload) - 1);
    if (xQueueSend(s_q, &m, 0) != pdTRUE) {
        s_dropped++;
        return false;
    }
    return true;
}

bool event_buffer_pop(event_msg_t *out, TickType_t timeout_ticks) {
    if (s_q == NULL || out == NULL) {
        return false;
    }
    return xQueueReceive(s_q, out, timeout_ticks) == pdTRUE;
}

uint32_t event_buffer_dropped(void) { return s_dropped; }
uint32_t event_buffer_depth(void) { return s_q == NULL ? 0 : (uint32_t)uxQueueMessagesWaiting(s_q); }
void event_buffer_reset_stats(void) { s_dropped = 0; }
