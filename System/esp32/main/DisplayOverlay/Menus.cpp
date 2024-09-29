#include "Menus.h"

#include "DisplayOverlay.h"
#include "WiFiMenu.h"
#include "BluetoothMenu.h"

static WiFiMenu      wifiMenu;
static BluetoothMenu btMenu;

//////////////////////////////////////////////////////////////////////////////
// Main menu
//////////////////////////////////////////////////////////////////////////////
class MainMenu : public Menu {
public:
    MainMenu() : Menu("Aquarius+", 20) {
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Wi-Fi");
            item.onEnter = []() { wifiMenu.show(); };
        }
        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Bluetooth");
            item.onEnter = []() { btMenu.show(); };
        }
        items.emplace_back(MenuItemType::subMenu, "Date/Time");

        items.emplace_back(MenuItemType::separator);
        items.emplace_back(MenuItemType::subMenu, "Exit core");
    }
};

static MainMenu mainMenu;

Menu *getMainMenu() {
    return &mainMenu;
}
