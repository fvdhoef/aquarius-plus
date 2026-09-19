#pragma once

#include "Menu.h"
#include <esp_ota_ops.h>
#include "FpgaCore.h"

#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wformat-truncation"
#endif

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

        const CoreInfo *coreInfo = FpgaCore::getCoreInfo();
        items.emplace_back(MenuItemType::separator, "FPGA core");
        snprintf(tmp, sizeof(tmp), "Name     :%s", coreInfo->name);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Version  :%u.%02u", coreInfo->versionMajor, coreInfo->versionMinor);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Core type:%02X", coreInfo->coreType);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Flags    :%02X", coreInfo->flags);
        items.emplace_back(MenuItemType::subMenu, tmp);
        items.emplace_back(MenuItemType::separator);

        items.emplace_back(MenuItemType::separator, "ESP");
        snprintf(tmp, sizeof(tmp), "Name   :%s", running_app_info.project_name);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Version:%s", (const char *)running_app_info.version);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Date   :%s", running_app_info.date);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Time   :%s", running_app_info.time);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "ESP-IDF:%s", (const char *)running_app_info.idf_ver);
        items.emplace_back(MenuItemType::subMenu, tmp);
    }
};

#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif
