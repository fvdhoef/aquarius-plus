#pragma once

#include "Menu.h"
#include <nvs_flash.h>
#include "Keyboard.h"

class SdCardUpdateMenu : public Menu {
public:
    SdCardUpdateMenu() : Menu("SdCard Update", 38) {
    }

    void onEnter() override;

private:
    void doUpdate();
};
