#pragma once

#include "Menu.h"
#include "WiFiMenu.h"
#include "BluetoothMenu.h"

static WiFiMenu      wifiMenu;
static BluetoothMenu btMenu;

//////////////////////////////////////////////////////////////////////////////
// Esp settings menu
//////////////////////////////////////////////////////////////////////////////
class EspSettingsMenu : public Menu {
public:
    EspSettingsMenu() : Menu("ESP settings", 38) {
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Wi-Fi");
            item.onEnter = []() { wifiMenu.show(); };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Bluetooth");
            item.onEnter = []() { btMenu.show(); };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Time zone: UTC");
            item.onEnter = []() {};
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Keyboard layout: FR/BE (AZERTY)");
            item.onEnter = []() {};
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "System update");
            item.onEnter = []() {};
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Factory reset");
            item.onEnter = []() {};
        }
    }
};
