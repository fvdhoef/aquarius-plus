#include "SdCardUpdateMenu.h"

#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <esp_ota_ops.h>
#include <nvs_flash.h>
#include "VFS.h"

void SdCardUpdateMenu::onEnter() {
    doUpdate();
    setExitMenu();
}

void SdCardUpdateMenu::doUpdate() {
    const int              app_desc_offset  = sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t);
    size_t                 update_size      = 0;
    const esp_partition_t *update_partition = nullptr;
    esp_err_t              err;

    esp_ota_handle_t ota_handle  = 0;
    int              fd          = -1;
    const size_t     tmpbuf_size = 65536;
    void            *tmpbuf      = malloc(tmpbuf_size);
    bool             success     = false;
    auto             vfs         = getSDCardVFS();

    if (tmpbuf == nullptr) {
        goto done;
    }

    struct stat st;
    if (vfs->stat(CONFIG_UPDATE_FILE_NAME, &st) < 0) {
        drawMessage(CONFIG_UPDATE_FILE_NAME " not found");
        vTaskDelay(pdMS_TO_TICKS(2000));
        goto done;
    }

    fd = vfs->open(FO_RDONLY, CONFIG_UPDATE_FILE_NAME);
    if (fd < 0) {
        drawMessage(CONFIG_UPDATE_FILE_NAME " not found");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    update_size = st.st_size;
    esp_app_desc_t running_app_info;
    if (esp_ota_get_partition_description(esp_ota_get_running_partition(), &running_app_info) != ESP_OK) {
        goto done;
    }

    esp_app_desc_t app_info;

    if (vfs->seek(fd, app_desc_offset) < 0)
        goto done;
    if (vfs->read(fd, sizeof(app_info), &app_info) != sizeof(app_info))
        goto done;

    if (app_info.magic_word != ESP_APP_DESC_MAGIC_WORD) {
        ESP_LOGE("update", "Incorrect app descriptor magic");
        drawMessage("Invalid update file");
        vTaskDelay(pdMS_TO_TICKS(2000));
        goto done;
    }

    if (strcmp(running_app_info.project_name, app_info.project_name) != 0) {
        ESP_LOGE("update", "Project name does not match");
        drawMessage("Invalid update file");
        vTaskDelay(pdMS_TO_TICKS(2000));
        goto done;
    }

    if ((update_partition = esp_ota_get_next_update_partition(nullptr)) == nullptr) {
        drawMessage("Can't find update partition");
        vTaskDelay(pdMS_TO_TICKS(2000));
        return;
    }

    drawMessage("Updating...");
    err = esp_ota_begin(update_partition, update_size, &ota_handle);
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_begin: %s", esp_err_to_name(err));
        goto done;
    }

    vfs->seek(fd, 0);

    while (1) {
        int size = vfs->read(fd, tmpbuf_size, tmpbuf);
        if (size <= 0)
            break;

        if ((err = esp_ota_write(ota_handle, tmpbuf, size)) != ESP_OK) {
            ESP_LOGE("update", "esp_ota_write: %s", esp_err_to_name(err));
            goto done;
        }
    }

    err        = esp_ota_end(ota_handle);
    ota_handle = 0;
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_end: %s", esp_err_to_name(err));
        goto done;
    }

    err = esp_ota_set_boot_partition(update_partition);
    if (err != ESP_OK) {
        ESP_LOGE("update", "esp_ota_set_boot_partition: %s", esp_err_to_name(err));
        if (err == ESP_ERR_OTA_VALIDATE_FAILED) {
            drawMessage("Invalid update file");
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
        goto done;
    }

    drawMessage("Update successful.");
    vTaskDelay(pdMS_TO_TICKS(2000));
    SystemRestart();

done:
    if (fd >= 0)
        vfs->close(fd);

    if (!success) {
        drawMessage("Upgrade failed.");
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
    if (ota_handle)
        esp_ota_abort(ota_handle);
}
