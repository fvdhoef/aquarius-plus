#pragma once

#include "Menu.h"
#include <esp_ota_ops.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"

class VersionMenu : public Menu {
public:
    VersionMenu() : Menu("Version", 38) {
        const esp_partition_t *running = esp_ota_get_running_partition();
        esp_app_desc_t         running_app_info;
        if (esp_ota_get_partition_description(running, &running_app_info) != ESP_OK) {
            setExitMenu();
            return;
        }

        char tmp[37];
        snprintf(tmp, sizeof(tmp), "Name:%s", running_app_info.project_name);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Version:%s", (const char *)running_app_info.version);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Compile date:%s", running_app_info.date);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Compile time:%s", running_app_info.time);
        items.emplace_back(MenuItemType::subMenu, tmp);
    }
};

#pragma GCC diagnostic pop
