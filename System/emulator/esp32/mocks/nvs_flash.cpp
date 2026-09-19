#include "nvs_flash.h"
#include "Config.h"

esp_err_t nvs_flash_erase() {
    auto config = Config::instance();
    config->nvs_u8.clear();
    return ESP_OK;
}

esp_err_t nvs_flash_init() {
    return ESP_OK;
}

esp_err_t nvs_open(const char *namespace_name, nvs_open_mode_t open_mode, nvs_handle_t *out_handle) {
    return ESP_OK;
}

void nvs_close(nvs_handle_t handle) {
}

esp_err_t nvs_commit(nvs_handle_t handle) {
    return ESP_OK;
}

esp_err_t nvs_get_u8(nvs_handle_t handle, const char *key, uint8_t *out_value) {
    auto config = Config::instance();
    auto it     = config->nvs_u8.find(key);
    if (it == config->nvs_u8.end())
        return ESP_FAIL;

    if (out_value)
        *out_value = it->second;
    return ESP_OK;
}

esp_err_t nvs_set_u8(nvs_handle_t handle, const char *key, uint8_t value) {
    auto config = Config::instance();
    config->nvs_u8.insert_or_assign(key, value);
    return ESP_OK;
}

esp_err_t nvs_get_blob(nvs_handle_t handle, const char *key, void *out_value, size_t *length) {
    auto config = Config::instance();
    auto it     = config->nvs_blobs.find(key);
    if (it == config->nvs_blobs.end())
        return ESP_FAIL;

    if (it->second.size() > *length)
        return ESP_ERR_NVS_INVALID_LENGTH;

    memcpy(out_value, it->second.data(), it->second.size());
    *length = it->second.size();
    return ESP_OK;
}

esp_err_t nvs_set_blob(nvs_handle_t handle, const char *key, const void *value, size_t length) {
    auto config = Config::instance();
    config->nvs_blobs.insert_or_assign(key, std::vector<uint8_t>((const uint8_t *)value, (const uint8_t *)value + length));
    return ESP_OK;
}

esp_err_t nvs_get_str(nvs_handle_t handle, const char *key, char *out_value, size_t *length) {
    return nvs_get_blob(handle, key, out_value, length);
}

esp_err_t nvs_set_str(nvs_handle_t handle, const char *key, const char *value) {
    return nvs_set_blob(handle, key, value, strlen(value) + 1);
}
