#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

class GamepadKeyboardMappingMenu : public Menu {
public:
    std::function<void()> onChange;
    std::function<void()> onLoad;
    std::function<void()> onSave;

    GamepadKeyboardMappingMenu() : Menu("Gamepad to keyboard mapping", 38) {
    }

    struct Button {
        const char *name;
        uint8_t     buttonIdx;
    };

    Button buttons[15] = {
        {"Up", GCB_DPAD_UP_IDX},
        {"Down", GCB_DPAD_DOWN_IDX},
        {"Left", GCB_DPAD_LEFT_IDX},
        {"Right", GCB_DPAD_RIGHT_IDX},
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

    bool    enabled             = false;
    uint8_t buttonScanCodes[16] = {0};

    void onUpdate() override {
        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Enable");
            item.setter = [this](int newVal) { enabled = newVal != 0; onChange(); };
            item.getter = [this]() { return enabled; };
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
            auto    buttonIdx = button.buttonIdx;
            char    assigned[20];
            uint8_t scanCode = buttonScanCodes[buttonIdx];

            if (scanCode == 0) {
                snprintf(assigned, sizeof(assigned), "Unassigned");
            } else {
                auto name = getScanCodeName(scanCode);
                if (name == nullptr) {
                    snprintf(assigned, sizeof(assigned), "Scancode %u", scanCode);
                } else {
                    snprintf(assigned, sizeof(assigned), "%s", name);
                }
            }

            char tmp[37];
            snprintf(tmp, sizeof(tmp), "%-5s -> %s", button.name, assigned);
            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [this, buttonIdx]() {
                drawMessage("Press key or ESC to unassign");
                int scanCode = Keyboard::instance()->waitScanCode();
                Keyboard::instance()->getKey(pdMS_TO_TICKS(100));
                if (scanCode == SCANCODE_ESCAPE) {
                    buttonScanCodes[buttonIdx] = 0;
                } else {
                    buttonScanCodes[buttonIdx] = scanCode;
                }
                onChange();
                setNeedsUpdate();
            };
        }
    }
};
