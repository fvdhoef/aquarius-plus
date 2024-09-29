#pragma once

#include "Menu.h"
#include "Bluetooth.h"

class BtDetailsMenu : public Menu {
public:
    BtDetailsMenu(const BtDevInfo &_bdi)
        : Menu("", 38), bdi(_bdi) {
        title = bdi.name;

        {
            char str[32];
            snprintf(
                str, sizeof(str), "Addr(%d) %02x:%02x:%02x:%02x:%02x:%02x",
                bdi.addr.type, bdi.addr.val[5], bdi.addr.val[4], bdi.addr.val[3], bdi.addr.val[2], bdi.addr.val[1], bdi.addr.val[0]);

            items.emplace_back(MenuItemType::subMenu, (const char *)str);
        }
        {
            const char *appearanceStr = nullptr;
            switch (bdi.appearance) {
                case 0x3C1: appearanceStr = "Keyboard"; break;
                case 0x3C2: appearanceStr = "Mouse"; break;
                case 0x3C3: appearanceStr = "Joystick"; break;
                case 0x3C4: appearanceStr = "Gamepad"; break;
            }

            if (appearanceStr) {
                char str[32];
                snprintf(str, sizeof(str), "Appearance: %s", appearanceStr);
                items.emplace_back(MenuItemType::subMenu, (const char *)str);
            }
        }

        items.emplace_back(MenuItemType::separator);

        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Forget device");
            item.onEnter = [&]() {
                getBluetooth()->forgetDevice(bdi.addr);
                needsUpdate = true;
                exitMenu    = true;
            };
        }
    }

    const BtDevInfo &bdi;
    bool             showConnect;
    bool             needsUpdate = false;
};

class BluetoothMenu : public Menu {

    BluetoothInfo bi;

public:
    BluetoothMenu() : Menu("", 38) {
    }

    MenuItem &addNetworkItem(const BtDevInfo &info) {
        // Map RSSI to signal strength characters
        char rssiChar = 2;
        if (info.rssi <= -128)
            rssiChar = 2; // Offline
        else if (info.rssi < -90)
            rssiChar = 3; // Online, no bars
        else if (info.rssi < -80)
            rssiChar = 4; // Online, 1 bars
        else if (info.rssi < -70)
            rssiChar = 5; // Online, 2 bars
        else if (info.rssi < -55)
            rssiChar = 6; // Online, 3 bars
        else
            rssiChar = 7; // Online, 4 bars

        char tmp[36];
        snprintf(tmp, sizeof(tmp), "%-32s  %c", info.name.substr(0, 32).c_str(), rssiChar);

        return items.emplace_back(MenuItemType::subMenu, tmp);
    }

    void updateMenu() {
        bi.connectedDevices.clear();
        bi.knownDevices.clear();
        bi.otherDevices.clear();

        auto bt      = getBluetooth();
        bi           = bt->getBluetoothInfo();
        bool enabled = bt->getEnabled();

        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Bluetooth");
            item.setter = [&](int newVal) {
                getBluetooth()->setEnabled(newVal != 0);
                updateMenu();
            };
            item.getter = []() { return getBluetooth()->getEnabled() ? 1 : 0; };
        }
        items.emplace_back(MenuItemType::separator);

        if (!bi.connectedDevices.empty()) {
            items.emplace_back(MenuItemType::separator, "Connected devices");
            for (auto &dev : bi.connectedDevices) {
                auto &item   = addNetworkItem(dev);
                item.onEnter = [&]() {
                    BtDetailsMenu detailsMenu(dev);
                    detailsMenu.show();
                    if (detailsMenu.needsUpdate)
                        updateMenu();
                };
            }
            items.emplace_back(MenuItemType::separator);
        }

        if (!bi.knownDevices.empty()) {
            items.emplace_back(MenuItemType::separator, "Known devices");
            for (auto &dev : bi.knownDevices) {
                auto &item   = addNetworkItem(dev);
                item.onEnter = [&]() {
                    BtDetailsMenu detailsMenu(dev);
                    detailsMenu.show();
                    if (detailsMenu.needsUpdate)
                        updateMenu();
                };
            }
            items.emplace_back(MenuItemType::separator);
        }

        if (!bi.otherDevices.empty()) {
            items.emplace_back(MenuItemType::separator, "Detected devices");
            for (auto &dev : bi.otherDevices) {
                auto &item   = addNetworkItem(dev);
                item.onEnter = [&]() {
                    std::string name = dev.name;
                    if (editString("Enter name for this device:", name) && !name.empty()) {
                        if (!getBluetooth()->addDevice(dev.addr, name)) {
                            drawMessage("Failed, forget a device first.");
                            vTaskDelay(pdMS_TO_TICKS(2000));
                        }
                        updateMenu();
                    }
                };
            }
        }

        while (items.back().type == MenuItemType::separator) {
            items.pop_back();
        }
    }

    void onEnter() override {
        updateMenu();
    }

    void onExit() override {
    }

    bool onTick() override {
        updateMenu();
        // if (++requestScanTicks >= 5) {
        //     requestScanTicks = 0;
        //     getWiFi()->updateInfo();
        // }
        return true;
    }
};
