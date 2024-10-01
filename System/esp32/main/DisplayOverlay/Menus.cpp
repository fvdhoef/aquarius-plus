#include "Menus.h"
#include "FpgaCore.h"

#include "DisplayOverlay.h"
#include "EspSettingsMenu.h"

static EspSettingsMenu espSettingsMenu;

//////////////////////////////////////////////////////////////////////////////
// Main menu
//////////////////////////////////////////////////////////////////////////////
class MainMenu : public Menu {
public:
    MainMenu() : Menu("", 38) {}

    void onUpdate() override {
        // Update title
        {
            time_t now;
            time(&now);
            struct tm timeinfo = *localtime(&now);

            char strftime_buf[20];
            strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);

            char tmp[40];
            snprintf(tmp, sizeof(tmp), "Aquarius+        %s", strftime_buf);
            title = tmp;
        }

        items.clear();

        auto fpgaCore = getFpgaCore();
        if (fpgaCore) {
            fpgaCore->addMainMenuItems(*this);
            items.emplace_back(MenuItemType::separator);
        }

        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Restart ESP (CTRL-ALT-ESC)");
            item.onEnter = [&]() {
                drawMessage("Restarting ESP...");
                esp_restart();
            };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "ESP settings");
            item.onEnter = []() { espSettingsMenu.show(); };
        }
    }

    bool onTick() override {
        setNeedsUpdate();
        return false;
    }
};

Menu *getMainMenu() {
    static MainMenu obj;
    return &obj;
}
