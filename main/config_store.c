#include "config_store.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_err.h"
#include <stdbool.h>

static nvs_handle_t s_nvs;
static bool s_ready;

void config_store_init(void) {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err == ESP_OK && nvs_open("rf_cfg", NVS_READWRITE, &s_nvs) == ESP_OK) {
        s_ready = true;
    }
}

int config_store_set_i32(const char *key, int32_t value) {
    if (!s_ready) return -1;
    return nvs_set_i32(s_nvs, key, value) == ESP_OK && nvs_commit(s_nvs) == ESP_OK ? 0 : -1;
}

int config_store_get_i32(const char *key, int32_t *out, int32_t default_value) {
    if (out == NULL) return -1;
    if (!s_ready || nvs_get_i32(s_nvs, key, out) != ESP_OK) *out = default_value;
    return 0;
}
