#pragma once

#include "Menu.h"
#include <nvs_flash.h>
#include "Keyboard.h"

class KeyboardLayoutMenu : public Menu {
public:
    KeyboardLayoutMenu() : Menu("Select keyboard layout", 38) {
    }

    void onEnter() override {
        for (int i = 0; i < (int)KeyLayout::Count; i++)
            addLayout(Keyboard::instance()->getKeyLayoutName((KeyLayout)i).c_str(), (KeyLayout)i);
    }

    void addLayout(const char *title, KeyLayout keyLayout) {
        auto &item   = items.emplace_back(MenuItemType::subMenu, title);
        item.onEnter = [this, keyLayout]() {
            Keyboard::instance()->setKeyLayout(keyLayout);

            // Save layout to flash
            {
                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_u8(h, "kblayout", (unsigned)Keyboard::instance()->getKeyLayout()) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            }

            setExitMenu();
        };
    }
};
