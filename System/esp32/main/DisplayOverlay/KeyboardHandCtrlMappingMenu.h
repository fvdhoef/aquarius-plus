#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

class KeyboardHandCtrlMappingMenu : public Menu {
public:
    std::function<void()> onChange;

    KeyboardHandCtrlMappingMenu() : Menu("Keyboard to hand ctrl mapping", 38) {
    }

    bool    enabled            = false;
    uint8_t buttonScanCodes[6] = {0};

    void onUpdate() override {
        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Enable");
            item.setter = [this](int newVal) { enabled = newVal != 0; onChange(); };
            item.getter = [this]() { return enabled; };
        }
        items.emplace_back(MenuItemType::separator);

        for (int i = 0; i < 6; i++) {
            char assigned[20];

            if (buttonScanCodes[i] == 0) {
                snprintf(assigned, sizeof(assigned), "Unassigned");
            } else {

                snprintf(assigned, sizeof(assigned), "%s", getScanCodeName(buttonScanCodes[i]));
            }

            char tmp[37];
            snprintf(tmp, sizeof(tmp), "%-11s -> HC1 Button %d", assigned, i + 1);
            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [this, i]() {
                drawMessage("Press key or ESC to unassign");
                int scanCode = getKeyboard()->waitScanCode();
                getKeyboard()->getKey(pdMS_TO_TICKS(100));
                if (scanCode == SCANCODE_ESCAPE) {
                    buttonScanCodes[i] = 0;
                } else {
                    buttonScanCodes[i] = scanCode;
                }

                onChange();
                setNeedsUpdate();
            };
        }
    }
};
