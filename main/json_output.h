#pragma once
#include <stdbool.h>
#include <stdint.h>

void json_output_init(void);
void json_send_line(const char *line);
void json_send_error(const char *error, const char *message, const char *command);
void json_send_unsupported(const char *feature);
uint32_t json_now_ms(void);
