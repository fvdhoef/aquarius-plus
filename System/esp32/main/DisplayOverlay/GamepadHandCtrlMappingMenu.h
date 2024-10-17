#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

class GamepadHandCtrlMappingMenu : public Menu {
public:
    std::function<void()> onChange;

    GamepadHandCtrlMappingMenu() : Menu("Gamepad to hand ctrl mapping", 38) {
    }

    struct Button {
        const char *name;
        uint8_t     buttonIdx;
    };

    Button buttons[11] = {
        {"A", GCB_A_IDX},
        {"B", GCB_B_IDX},
        {"X", GCB_X_IDX},
        {"Y", GCB_Y_IDX},
        {"LB", GCB_LB_IDX},
        {"RB", GCB_RB_IDX},
        {"LS", GCB_LS_IDX},
        {"RS", GCB_RS_IDX},
        {"View", GCB_VIEW_IDX},
        {"Menu", GCB_MENU_IDX},
        {"Share", GCB_SHARE_IDX},
    };

    bool    enabled          = false;
    uint8_t buttonNumber[16] = {0};

    void onUpdate() override {
        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Enable");
            item.setter = [this](int newVal) { enabled = newVal != 0; onChange(); };
            item.getter = [this]() { return enabled; };
        }
        items.emplace_back(MenuItemType::separator);

        for (auto &button : buttons) {
            auto buttonIdx = button.buttonIdx;
            char assigned[20];

            if (buttonNumber[buttonIdx] == 0) {
                snprintf(assigned, sizeof(assigned), "Unassigned");
            } else {
                snprintf(assigned, sizeof(assigned), "%u", buttonNumber[buttonIdx]);
            }

            char tmp[37];
            snprintf(tmp, sizeof(tmp), "%-5s -> %s", button.name, assigned);
            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [this, buttonIdx]() {
                drawMessage("Press 1-6 or ESC to unassign");
                int ch = getKeyboard()->getKey(portMAX_DELAY);
                if (ch == 3) {
                    buttonNumber[buttonIdx] = 0;
                } else if (ch >= '1' && ch <= '6') {
                    buttonNumber[buttonIdx] = ch - '0';
                }
                onChange();
                setNeedsUpdate();
            };
        }
    }
};
