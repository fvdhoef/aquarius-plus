#pragma once

#include "Menu.h"
#include "Keyboard.h"
#include "GameCtrl.h"

class FileListMenu : public Menu {
public:
    std::string                              path;
    std::function<void(const std::string &)> onSelect;
    DirEnumCtx                               ctx;

    FileListMenu() : Menu("", 38) {
    }

    bool    enabled            = false;
    uint8_t buttonScanCodes[6] = {0};

    void onUpdate() override {
        items.clear();

        auto vfs     = getSDCardVFS();
        auto entries = vfs->direnum(path, 0);
        if (entries.first == 0) {
            ctx = entries.second;

            for (auto &entry : *ctx) {
                if (entry.attr & DE_ATTR_DIR)
                    continue;

                std::string *fileName = &entry.filename;
                auto        &item     = items.emplace_back(MenuItemType::subMenu, *fileName);
                item.onEnter          = [this, fileName]() {
                    setExitMenu();
                    onSelect(path + '/' + *fileName);
                };
            }
        }

        if (items.empty()) {
            items.emplace_back(MenuItemType::onOff, "No presets found");
        }
    }
};
