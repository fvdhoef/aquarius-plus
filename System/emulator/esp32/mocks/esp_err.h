#pragma once

typedef int esp_err_t;

#define ESP_OK   0
#define ESP_FAIL (-1)

#define ESP_ERROR_CHECK(x) (x)

#define ESP_ERR_NVS_BASE              0x1100
#define ESP_ERR_NVS_INVALID_LENGTH    (ESP_ERR_NVS_BASE + 0x0c)
#define ESP_ERR_NVS_NO_FREE_PAGES     (ESP_ERR_NVS_BASE + 0x0d)
#define ESP_ERR_NVS_NEW_VERSION_FOUND (ESP_ERR_NVS_BASE + 0x10)