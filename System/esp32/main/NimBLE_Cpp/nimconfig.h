#pragma once

#define CONFIG_NIMBLE_CPP_LOG_LEVEL ESP_LOG_INFO

#include <nimble/nimble_port.h>
#include <nimble/nimble_port_freertos.h>
#include <host/ble_hs.h>
#include <host/util/util.h>
#include <services/gap/ble_svc_gap.h>

#include "esp_log.h"
#ifndef CONFIG_NIMBLE_CPP_LOG_LEVEL
#    define CONFIG_NIMBLE_CPP_LOG_LEVEL 0
#endif

#define NIMBLE_CPP_LOG_PRINT(level, tag, format, ...)               \
    do {                                                            \
        if (CONFIG_NIMBLE_CPP_LOG_LEVEL >= level)                   \
            ESP_LOG_LEVEL_LOCAL(level, tag, format, ##__VA_ARGS__); \
    } while (0)

#define NIMBLE_LOGD(tag, format, ...) NIMBLE_CPP_LOG_PRINT(ESP_LOG_DEBUG, tag, format, ##__VA_ARGS__)
#define NIMBLE_LOGI(tag, format, ...) NIMBLE_CPP_LOG_PRINT(ESP_LOG_INFO, tag, format, ##__VA_ARGS__)
#define NIMBLE_LOGW(tag, format, ...) NIMBLE_CPP_LOG_PRINT(ESP_LOG_WARN, tag, format, ##__VA_ARGS__)
#define NIMBLE_LOGE(tag, format, ...) NIMBLE_CPP_LOG_PRINT(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)
#define NIMBLE_LOGC(tag, format, ...) NIMBLE_CPP_LOG_PRINT(ESP_LOG_ERROR, tag, format, ##__VA_ARGS__)
