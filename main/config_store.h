#pragma once
#include <stdint.h>

void config_store_init(void);
int config_store_set_i32(const char *key, int32_t value);
int config_store_get_i32(const char *key, int32_t *out, int32_t default_value);
