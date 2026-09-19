#pragma once

#include <stdint.h>
#include "esp_err.h"

typedef struct {
} esp_partition_t;

const esp_partition_t *esp_ota_get_running_partition();

typedef struct {
    char version[32];
    char project_name[32];
    char time[16];
    char date[16];
    char idf_ver[32];
} esp_app_desc_t;

esp_err_t esp_ota_get_partition_description(const esp_partition_t *partition, esp_app_desc_t *app_desc);
