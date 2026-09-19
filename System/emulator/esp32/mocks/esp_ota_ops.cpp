#include "esp_ota_ops.h"
#include <stdio.h>
#include <string.h>

const esp_partition_t *esp_ota_get_running_partition() {
    return nullptr;
}

esp_err_t esp_ota_get_partition_description(const esp_partition_t *partition, esp_app_desc_t *app_desc) {
    extern const char *versionStr;

    memset(app_desc, 0, sizeof(*app_desc));
    snprintf(app_desc->project_name, sizeof(app_desc->project_name), "aquarius-plus");
    snprintf(app_desc->version, sizeof(app_desc->version), "%s", versionStr);
    snprintf(app_desc->date, sizeof(app_desc->date), "%s", __DATE__);
    snprintf(app_desc->time, sizeof(app_desc->time), "%s", __TIME__);
    snprintf(app_desc->idf_ver, sizeof(app_desc->idf_ver), "-");

    return ESP_OK;
}
