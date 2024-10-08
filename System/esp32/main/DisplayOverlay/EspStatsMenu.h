#pragma once

#include "Menu.h"
#include <esp_heap_caps.h>

class EspStatsMenu : public Menu {
public:
    EspStatsMenu() : Menu("ESP stats", 38) {
    }

    void onUpdate() override {
        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

        setNeedsRedraw();
        items.clear();

        items.emplace_back(MenuItemType::separator, "Memory");
        char tmp[40];
        snprintf(tmp, sizeof(tmp), "Total              %7u bytes", (unsigned)(info.total_free_bytes + info.total_allocated_bytes));
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Free               %7u bytes", (unsigned)info.total_free_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Allocated          %7u bytes", (unsigned)info.total_allocated_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Largest free       %7u bytes", (unsigned)info.largest_free_block);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Minimum free       %7u bytes", (unsigned)info.minimum_free_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Allocated blocks   %7u", (unsigned)info.allocated_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Free blocks        %7u", (unsigned)info.free_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Total blocks       %7u", (unsigned)info.total_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);

        nvs_stats_t stats;
        if (nvs_get_stats(nullptr, &stats) == ESP_OK) {
            items.emplace_back(MenuItemType::separator);
            items.emplace_back(MenuItemType::separator, "NVS");

            snprintf(tmp, sizeof(tmp), "Used entries       %7u", stats.used_entries);
            items.emplace_back(MenuItemType::subMenu, tmp);
            snprintf(tmp, sizeof(tmp), "Free entries       %7u", stats.free_entries);
            items.emplace_back(MenuItemType::subMenu, tmp);
            snprintf(tmp, sizeof(tmp), "Available entries  %7u", stats.available_entries);
            items.emplace_back(MenuItemType::subMenu, tmp);
            snprintf(tmp, sizeof(tmp), "Total entries      %7u", stats.total_entries);
            items.emplace_back(MenuItemType::subMenu, tmp);
            snprintf(tmp, sizeof(tmp), "Namespace count    %7u", stats.namespace_count);
            items.emplace_back(MenuItemType::subMenu, tmp);
        }
    }

    bool onTick() override {
        setNeedsUpdate();
        return false;
    }
};
