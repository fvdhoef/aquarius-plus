#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

struct Kb2HcMapping {
    uint8_t enabled            = false;
    uint8_t buttonScanCodes[6] = {
        SCANCODE_INSERT,
        SCANCODE_HOME,
        SCANCODE_PAGEUP,
        SCANCODE_DELETE,
        SCANCODE_END,
        SCANCODE_PAGEDOWN,
    };
};

class KeyboardHandCtrlMappingMenu : public Menu {
public:
    std::function<void()> onChange;
    std::function<void()> onLoad;
    std::function<void()> onSave;

    KeyboardHandCtrlMappingMenu() : Menu("Keyboard to hand ctrl mapping", 38) {
    }

    Kb2HcMapping settings;

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

        for (int i = 0; i < 6; i++) {
            char assigned[20];

            if (settings.buttonScanCodes[i] == 0) {
                snprintf(assigned, sizeof(assigned), "Unassigned");
            } else {

                snprintf(assigned, sizeof(assigned), "%s", getScanCodeName(settings.buttonScanCodes[i]));
            }

            char tmp[37];
            snprintf(tmp, sizeof(tmp), "%-11s -> HC1 Button %d", assigned, i + 1);
            auto &item   = items.emplace_back(MenuItemType::subMenu, tmp);
            item.onEnter = [this, i]() {
                drawMessage("Press key or ESC to unassign");
                int scanCode = Keyboard::instance()->waitScanCode();
                Keyboard::instance()->getKey(pdMS_TO_TICKS(100));
                if (scanCode == SCANCODE_ESCAPE) {
                    settings.buttonScanCodes[i] = 0;
                } else {
                    settings.buttonScanCodes[i] = scanCode;
                }

                onChange();
                setNeedsUpdate();
            };
        }
    }
};
