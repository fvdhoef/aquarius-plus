#pragma once

#include "Menu.h"
#include <esp_heap_caps.h>

class EspStatsMenu : public Menu {
public:
    EspStatsMenu() : Menu("ESP stats", 26) {
    }

    void onUpdate() override {
        multi_heap_info_t info;
        heap_caps_get_info(&info, MALLOC_CAP_DEFAULT);

        setNeedsRedraw();
        items.clear();
        char tmp[40];
        snprintf(tmp, sizeof(tmp), "Total            %7u", (unsigned)(info.total_free_bytes + info.total_allocated_bytes));
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Free             %7u", (unsigned)info.total_free_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Allocated        %7u", (unsigned)info.total_allocated_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Largest free     %7u", (unsigned)info.largest_free_block);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Minimum free     %7u", (unsigned)info.minimum_free_bytes);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Allocated blocks %7u", (unsigned)info.allocated_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Free blocks      %7u", (unsigned)info.free_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);
        snprintf(tmp, sizeof(tmp), "Total blocks     %7u", (unsigned)info.total_blocks);
        items.emplace_back(MenuItemType::subMenu, tmp);
    }

    bool onTick() override {
        setNeedsUpdate();
        return false;
    }
};
