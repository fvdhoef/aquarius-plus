#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

struct Gp2HcMapping {
    bool    enabled          = true;
    uint8_t buttonNumber[16] = {
        1, // GCB_A_IDX
        2, // GCB_B_IDX
        3, // GCB_X_IDX
        4, // GCB_Y_IDX
        0, // GCB_VIEW_IDX
        0, // GCB_GUIDE_IDX
        0, // GCB_MENU_IDX
        0, // GCB_LS_IDX
        0, // GCB_RS_IDX
        5, // GCB_LB_IDX
        6, // GCB_RB_IDX
        0, // GCB_DPAD_UP_IDX
        0, // GCB_DPAD_DOWN_IDX
        0, // GCB_DPAD_LEFT_IDX
        0, // GCB_DPAD_RIGHT_IDX
        0, // GCB_SHARE_IDX
    };
};

class GamepadHandCtrlMappingMenu : public Menu {
public:
    std::function<void()> onChange;
    std::function<void()> onLoad;
    std::function<void()> onSave;

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

    Gp2HcMapping settings;

    void onUpdate() override {
        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Enable");
            item.setter = [this](int newVal) { settings.enabled = newVal != 0; onChange(); };
            item.getter = [this]() { return settings.enabled; };
        }
        items.emplace_back(MenuItemType::separator);
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Load preset");
            item.onEnter = [this]() { onLoad(); setNeedsUpdate(); };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Save preset");
            item.onEnter = [this]() { onSave(); };
        }
        items.emplace_back(MenuItemType::separator);

        for (auto &button : buttons) {
            auto buttonIdx = button.buttonIdx;
            char assigned[20];

            if (settings.buttonNumber[buttonIdx] == 0) {
                snprintf(assigned, sizeof(assigned), "Unassigned");
            } else {
                snprintf(assigned, sizeof(assigned), "%u", settings.buttonNumber[buttonIdx]);
            }

            char tmp[37];
            snprintf(tmp, sizeof(tmp), "%-5s -> %s", button.name, assigned);
            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [this, buttonIdx]() {
                drawMessage("Press 1-6 or ESC to unassign");
                int ch = Keyboard::instance()->getKey(portMAX_DELAY);
                if (ch == 3) {
                    settings.buttonNumber[buttonIdx] = 0;
                } else if (ch >= '1' && ch <= '6') {
                    settings.buttonNumber[buttonIdx] = ch - '0';
                }
                onChange();
                setNeedsUpdate();
            };
        }
    }
};
