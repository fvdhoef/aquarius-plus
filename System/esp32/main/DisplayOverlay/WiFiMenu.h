#pragma once

#include "Menu.h"
#include "WiFi.h"
#include <nvs_flash.h>

class WiFiDetailsMenu : public Menu {
public:
    WiFiDetailsMenu(WiFiApInfo &_wai, NetworkInfo *_ni = nullptr, bool _showConnect = false)
        : Menu("", 38), wai(_wai), ni(_ni), showConnect(_showConnect) {
        title = _wai.ssid;

        if (ni) {
            items.emplace_back(MenuItemType::subMenu, "IP      " + ni->ip);
            items.emplace_back(MenuItemType::subMenu, "Netmask " + ni->netmask);
            items.emplace_back(MenuItemType::subMenu, "Gateway " + ni->gateway);
            if (!ni->dns1.empty())
                items.emplace_back(MenuItemType::subMenu, "DNS     " + ni->dns1);
            if (!ni->dns2.empty())
                items.emplace_back(MenuItemType::subMenu, "DNS     " + ni->dns2);
            if (!ni->dns3.empty())
                items.emplace_back(MenuItemType::subMenu, "DNS     " + ni->dns3);

            items.emplace_back(MenuItemType::separator);
        }

        if (showConnect) {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Join network");
            item.onEnter = [&]() {
                getWiFi()->joinNetwork(wai.ssid, "");
                needsUpdate = true;
                exitMenu    = true;
            };
        }

        {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Forget network");
            item.onEnter = [&]() {
                getWiFi()->forgetNetwork(wai.ssid);
                needsUpdate = true;
                exitMenu    = true;
            };
        }
    }

    WiFiApInfo  &wai;
    NetworkInfo *ni;
    bool         showConnect;
    bool         needsUpdate = false;
};

class WiFiMenu : public Menu {

    int         requestScanTicks = 0;
    NetworkInfo ni;

public:
    WiFiMenu() : Menu("", 38) {
    }

    MenuItem &addNetworkItem(const WiFiApInfo &info) {
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

        char authModeChar = (info.authMode == ' ' || info.authMode == 'O') ? ' ' : 1;

        char tmp[36];
        snprintf(tmp, sizeof(tmp), "%-32s %c%c", info.ssid.substr(0, 32).c_str(), authModeChar, rssiChar);

        return items.emplace_back(MenuItemType::subMenu, tmp);
    }

    void updateMenu() {
        auto wifi    = getWiFi();
        ni           = wifi->getNetworkInfo();
        bool enabled = wifi->getEnabled();

        items.clear();
        {
            auto &item  = items.emplace_back(MenuItemType::onOff, "Wi-Fi");
            item.setter = [&](int newVal) {
                getWiFi()->setEnabled(newVal != 0);
                updateMenu();
            };
            item.getter = []() { return getWiFi()->getEnabled() ? 1 : 0; };
        }
        items.emplace_back(MenuItemType::separator);

        {
            char str[32];
            snprintf(
                str, sizeof(str), "MAC      %02x:%02x:%02x:%02x:%02x:%02x",
                ni.mac[0], ni.mac[1], ni.mac[2], ni.mac[3], ni.mac[4], ni.mac[5]);
            items.emplace_back(MenuItemType::subMenu, str);
        }

        auto hostName = wifi->getHostName();
        if (!hostName.empty()) {
            auto &item   = items.emplace_back(MenuItemType::subMenu, "Hostname " + wifi->getHostName());
            item.onEnter = [this]() {
                std::string hostName = getWiFi()->getHostName();
                if (editString("Enter new hostname", hostName, 30) && !hostName.empty()) {
                    nvs_handle_t h;
                    if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                        if (nvs_set_str(h, "hostname", hostName.c_str()) == ESP_OK) {
                            nvs_commit(h);
                        }
                        nvs_close(h);
                    }

                    drawMessage("Restart ESP to use new hostname.");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                }
            };
        }
        items.emplace_back(MenuItemType::separator);

        if (ni.connected && !ni.currentNetwork.ssid.empty()) {
            items.emplace_back(MenuItemType::separator, "Current network");
            auto &item   = addNetworkItem(ni.currentNetwork);
            item.onEnter = [&]() {
                WiFiDetailsMenu detailsMenu(ni.currentNetwork, &ni);
                detailsMenu.show();
                if (detailsMenu.needsUpdate)
                    updateMenu();
            };
            items.emplace_back(MenuItemType::separator);
        }

        if (!ni.knownNetworks.empty()) {
            items.emplace_back(MenuItemType::separator, "Known networks");
            for (auto &info : ni.knownNetworks) {
                auto &item   = addNetworkItem(info);
                item.onEnter = [&, enabled]() {
                    WiFiDetailsMenu detailsMenu(info, nullptr, enabled);
                    detailsMenu.show();
                    if (detailsMenu.needsUpdate)
                        updateMenu();
                };
            }
            items.emplace_back(MenuItemType::separator);
        }

        if (!ni.otherNetworks.empty()) {
            items.emplace_back(MenuItemType::separator, "Other networks");
            for (auto &info : ni.otherNetworks) {
                auto &item   = addNetworkItem(info);
                item.onEnter = [&]() {
                    std::string password;
                    if (info.authMode == 'O' || editString("Enter password:", password, 64)) {
                        getWiFi()->joinNetwork(info.ssid, password);
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
        getWiFi()->updateInfo();
        updateMenu();
    }

    void onExit() override {
    }

    bool onTick() override {
        updateMenu();
        if (++requestScanTicks >= 5) {
            requestScanTicks = 0;
            getWiFi()->updateInfo();
        }
        return true;
    }
};
