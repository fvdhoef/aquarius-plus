#include "GitHubUpdateMenu.h"

#include <esp_ota_ops.h>
#include <esp_app_format.h>
#include <esp_ota_ops.h>
#include <esp_https_ota.h>
#include <nvs_flash.h>

// trim from left
static inline std::string ltrim(const std::string &s, const char *t = " \t\n\r\f\v") {
    std::string result = s;
    result.erase(0, result.find_first_not_of(t));
    return result;
}

// trim from right
static inline std::string rtrim(const std::string &s, const char *t = " \t\n\r\f\v") {
    std::string result = s;
    result.erase(result.find_last_not_of(t) + 1);
    return result;
}

// trim from left & right
static inline std::string trim(const std::string &s, const char *t = " \t\n\r\f\v") {
    return ltrim(rtrim(s, t), t);
}

static esp_err_t http_evt_cb(esp_http_client_event_t *evt) {
    if (evt->event_id == HTTP_EVENT_ON_HEADER) {
        if (strcasecmp(evt->header_key, "Location") == 0) {
            char *tag = strrchr(evt->header_value, '/');
            if (tag) {
                tag += 1;
                char *latestTag;
                esp_http_client_get_user_data(evt->client, (void **)&latestTag);
                if (latestTag)
                    snprintf(latestTag, LATEST_TAG_LEN, "%s", tag);
            }
        }
    }
    return ESP_OK;
}

void GitHubUpdateMenu::onEnter() {
    drawMessage("Getting latest release");

    latestTag[0] = 0;

    // Get tag of latest release
    {
        esp_http_client_config_t cfg = {
            .url                   = CONFIG_GITHUB_BASE_URL "/releases/latest",
            .timeout_ms            = 5000,
            .disable_auto_redirect = true,
            .event_handler         = http_evt_cb,
            .user_data             = latestTag,
            .use_global_ca_store   = true,
        };
        esp_http_client_handle_t client = esp_http_client_init(&cfg);
        esp_err_t                err    = esp_http_client_open(client, 0);
        if (err == ESP_OK) {
            esp_http_client_fetch_headers(client);
            esp_http_client_close(client);
        }
        esp_http_client_cleanup(client);
    }

    if (!latestTag[0]) {
        items.emplace_back(MenuItemType::subMenu, "Error fetching latest version.");
    } else {
        auto &item   = items.emplace_back(MenuItemType::subMenu, latestTag);
        item.onEnter = [this]() {
            doUpdate(latestTag);
        };
    }
    items.emplace_back(MenuItemType::separator);
    {
        auto &item   = items.emplace_back(MenuItemType::subMenu, "Specific version");
        item.onEnter = [&]() {
            std::string tag = latestTag;
            if (!editString("Enter firmware version (GitHub tag)", tag)) {
                setExitMenu();
                return;
            }

            tag = trim(tag);
            if (tag.empty()) {
                setExitMenu();
                return;
            }
            if (tag[0] == 'v') {
                tag[0] = 'V';
            }

            doUpdate(tag.c_str());
        };
    }
}

void GitHubUpdateMenu::doUpdate(const char *tag) {
    auto url = std::string(CONFIG_GITHUB_BASE_URL "/releases/download/") + tag + "/" + CONFIG_UPDATE_FILE_NAME;

    esp_http_client_config_t http_config = {
        .url                 = url.c_str(),
        .timeout_ms          = 5000,
        .buffer_size         = 64 * 1024,
        .buffer_size_tx      = 10 * 1024,
        .use_global_ca_store = true,
        .keep_alive_enable   = true,
    };
    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_err_t              err;
    esp_https_ota_handle_t https_ota_handle = NULL;
    int                    lastPercentage   = -1;

    drawMessage("Updating...");

    err = esp_https_ota_begin(&ota_config, &https_ota_handle);
    if (err != ESP_OK) {
        drawMessage("Error starting OTA update");
        vTaskDelay(pdMS_TO_TICKS(2000));
        goto ota_end;
    }

    esp_app_desc_t app_desc;
    err = esp_https_ota_get_img_desc(https_ota_handle, &app_desc);
    if (err != ESP_OK) {
        drawMessage("Error getting image header");
        vTaskDelay(pdMS_TO_TICKS(2000));
        goto ota_end;
    }

    // Validate image header
    {
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t         running_app_info;
        if (esp_ota_get_partition_description(running, &running_app_info) != ESP_OK) {
            drawMessage("Error.");
            vTaskDelay(pdMS_TO_TICKS(2000));
            goto ota_end;
        }
        if (strcmp(app_desc.project_name, running_app_info.project_name) != 0) {
            drawMessage("Incompatible update, aborting.");
            vTaskDelay(pdMS_TO_TICKS(2000));
            goto ota_end;
        }
        if (memcmp(app_desc.version, running_app_info.version, sizeof(app_desc.version)) == 0 &&
            memcmp(app_desc.time, running_app_info.time, sizeof(app_desc.time)) == 0 &&
            memcmp(app_desc.date, running_app_info.date, sizeof(app_desc.date)) == 0) {

            drawMessage("Already updated, aborting.");
            vTaskDelay(pdMS_TO_TICKS(2000));
            goto ota_end;
        }
    }

    while (1) {
        err = esp_https_ota_perform(https_ota_handle);
        if (err != ESP_ERR_HTTPS_OTA_IN_PROGRESS) {
            break;
        }
        // esp_https_ota_perform returns after every read operation which gives user the ability to
        // monitor the status of OTA upgrade by calling esp_https_ota_get_image_len_read, which gives length of image
        // data read so far.
        auto progressSize = esp_https_ota_get_image_len_read(https_ota_handle);
        auto totalSize    = esp_https_ota_get_image_size(https_ota_handle);
        auto percentage   = (progressSize * 100) / totalSize;
        if (lastPercentage != percentage) {
            lastPercentage = percentage;

            char tmp[40];
            snprintf(tmp, sizeof(tmp), "Updating: %d%%", percentage);
            drawMessage(tmp);
        }
    }

    if (esp_https_ota_is_complete_data_received(https_ota_handle) != true) {
        drawMessage("Data incomplete, aborting.");
        vTaskDelay(pdMS_TO_TICKS(2000));
        setExitMenu();

    } else {
        esp_err_t ota_finish_err = esp_https_ota_finish(https_ota_handle);
        if (err == ESP_OK && ota_finish_err == ESP_OK) {
            drawMessage("Update successful. Rebooting.");
            vTaskDelay(pdMS_TO_TICKS(2000));
            esp_restart();

        } else {
            if (ota_finish_err == ESP_ERR_OTA_VALIDATE_FAILED) {
                drawMessage("Image validation failed.");
                vTaskDelay(pdMS_TO_TICKS(2000));
                setExitMenu();
            }
            return;
        }
    }
    return;

ota_end:
    if (https_ota_handle != NULL) {
        esp_https_ota_abort(https_ota_handle);
    }

    drawMessage("Upgrade failed.");
    vTaskDelay(pdMS_TO_TICKS(2000));
    setExitMenu();
}
