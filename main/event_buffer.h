#pragma once
#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "common.h"

void event_buffer_init(void);
bool event_buffer_push(const char *json_line);
bool event_buffer_pop(event_msg_t *out, TickType_t timeout_ticks);
uint32_t event_buffer_dropped(void);
uint32_t event_buffer_depth(void);
void event_buffer_reset_stats(void);
