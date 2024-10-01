#pragma once

#include "Menu.h"
#include "WiFiMenu.h"
#include "BluetoothMenu.h"
#include "TimeZoneMenu.h"
#include "KeyboardLayoutMenu.h"
#include "GitHubUpdateMenu.h"
#include "SdCardUpdateMenu.h"
#include "EspStatsMenu.h"

static WiFiMenu      wifiMenu;
static BluetoothMenu btMenu;

//////////////////////////////////////////////////////////////////////////////
// Esp settings menu
//////////////////////////////////////////////////////////////////////////////
class EspSettingsMenu : public Menu {
public:
    EspSettingsMenu() : Menu("ESP settings", 38) {
    }

    void onUpdate() override {
        items.clear();
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Wi-Fi");
            item.onEnter = []() { wifiMenu.show(); };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Bluetooth");
            item.onEnter = []() { btMenu.show(); };
        }
        items.emplace_back(MenuItemType::separator);
        {
            char tmp[40];
            snprintf(tmp, sizeof(tmp), "Time zone: %s", TimeZoneMenu::getTimeZone().c_str());

            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [&]() {
                TimeZoneMenu subMenu;
                subMenu.show();
                setNeedsUpdate();
            };
        }
        {
            char tmp[40];
            snprintf(tmp, sizeof(tmp), "Keyboard layout: %s", getKeyboard()->getKeyLayoutName(getKeyboard()->getKeyLayout()).c_str());

            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [&]() {
                KeyboardLayoutMenu subMenu;
                subMenu.show();
                setNeedsUpdate();
            };
        }
        items.emplace_back(MenuItemType::separator);
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "System update from GitHub");
            item.onEnter = []() {
                GitHubUpdateMenu subMenu;
                subMenu.show();
            };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "System update from SD card");
            item.onEnter = []() {
                SdCardUpdateMenu subMenu;
                subMenu.show();
            };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Factory reset");
            item.onEnter = [&]() {
                drawMessage("Erasing settings...");
                ESP_ERROR_CHECK(nvs_flash_erase());
                ESP_ERROR_CHECK(nvs_flash_init());
                drawMessage("Restarting system...");
                esp_restart();
            };
        }
        items.emplace_back(MenuItemType::separator);
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "ESP stats");
            item.onEnter = [&]() {
                EspStatsMenu subMenu;
                subMenu.show();
            };
        }
    }
};
