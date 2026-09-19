#pragma once

#include "Menu.h"
#include <nvs_flash.h>

class TimeZoneMenu : public Menu {
public:
    static std::string getTimeZone() {
        const char *cur_tz;

        cur_tz = getenv("TZ");
        if (!cur_tz)
            cur_tz = "";

        const char *p = cur_tz;
        if (p[0] == 0)
            return "No timezone set";

        std::string tzName;
        std::string tzDstName;

        // Extract timezone
        while (isalpha(*p))
            tzName.push_back(*(p++));

        // Skip GMT offset
        while (isdigit(*p) || *p == ':' || *p == '-')
            p++;

        // Extract DST timezone if available
        while (isalpha(*p))
            tzDstName.push_back(*(p++));

        if (tzDstName.empty())
            return tzName;

        return tzName + '/' + tzDstName;
    }

    int subMenu = 0;

    TimeZoneMenu() : Menu("Select time zone", 38) {
    }

    void onEnter() override {
        subMenu = 0;
    }

    void addTimeZone(const char *title, const char *tz) {
        auto &item   = items.emplace_back(MenuItemType::subMenu, title);
        item.onEnter = [this, title, tz]() {
            setenv("TZ", tz, 1);

            // Save timezone to flash
            {
                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_str(h, "tz", tz) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            }

            setExitMenu();
        };
    }

    void onUpdate() override {
        setNeedsRedraw();
        items.clear();
        switch (subMenu) {
            default: {
                {
                    auto &item   = items.emplace_back(MenuItemType::subMenu, "North/South America");
                    item.onEnter = [this]() { subMenu = 1; resetSelectedRow(); setNeedsUpdate(); };
                }
                {
                    auto &item   = items.emplace_back(MenuItemType::subMenu, "Europe");
                    item.onEnter = [this]() { subMenu = 2; resetSelectedRow(); setNeedsUpdate(); };
                }
                {
                    auto &item   = items.emplace_back(MenuItemType::subMenu, "Africa");
                    item.onEnter = [this]() { subMenu = 3; resetSelectedRow(); setNeedsUpdate(); };
                }
                {
                    auto &item   = items.emplace_back(MenuItemType::subMenu, "Asia");
                    item.onEnter = [this]() { subMenu = 4; resetSelectedRow(); setNeedsUpdate(); };
                }
                {
                    auto &item   = items.emplace_back(MenuItemType::subMenu, "Australia/New Zealand");
                    item.onEnter = [this]() { subMenu = 5; resetSelectedRow(); setNeedsUpdate(); };
                }
                break;
            }

            case 1: {
                addTimeZone("SST        Samoa", "SST11");
                addTimeZone("HST        Hawaii", "HST10");
                addTimeZone("HST/HDT    Hawaii", "HST10HDT,M3.2.0,M11.1.0");
                addTimeZone("AKST/AKDT  Alaska", "AKST9AKDT,M3.2.0,M11.1.0");
                addTimeZone("PST/PDT    Pacific", "PST8PDT,M3.2.0,M11.1.0");
                addTimeZone("MST        Mountain", "MST7");
                addTimeZone("MST/MDT    Mountain", "MST7MDT,M3.2.0,M11.1.0");
                addTimeZone("CST        Central", "CST6");
                addTimeZone("CST/CDT    Central", "CST6CDT,M3.2.0,M11.1.0");
                addTimeZone("EST        Eastern", "EST5");
                addTimeZone("EST/EDT    Eastern", "EST5EDT,M3.2.0,M11.1.0");
                addTimeZone("AST        Atlantic", "AST4");
                addTimeZone("AST/ADT    Atlantic", "AST4ADT,M3.2.0,M11.1.0");
                addTimeZone("NST/NDT    Newfoundland", "NST3:30NDT,M3.2.0,M11.1.0");
                break;
            }
            case 2: {
                addTimeZone("GMT       Greenwich Mean", "GMT0");
                addTimeZone("GMT/BST   UK", "GMT0BST,M3.5.0/1,M10.5.0");
                addTimeZone("GMT/IST   Ireland", "GMT0IST,M3.5.0/1,M10.5.0");
                addTimeZone("WET/WEST  Portugal", "WET0WEST,M3.5.0/1,M10.5.0");
                addTimeZone("CET/CEST  Most of Western Europe", "CET-1CEST,M3.5.0,M10.5.0/3");
                addTimeZone("EET       Eastern Europe", "EET-2");
                addTimeZone("EET/EEST  Eastern Europe", "EET-2EEST,M3.5.0/3,M10.5.0/4");
                addTimeZone("MSK       Western Russia", "MSK-3");
                break;
            }
            case 3: {
                addTimeZone("GMT   Greenwich Mean", "GMT0");
                addTimeZone("WAT   West Africa", "WAT-1");
                addTimeZone("SAST  South Africa", "SAST-2");
                addTimeZone("CAST  Central Africa", "CAT-2");
                addTimeZone("EAT   East Africa", "EAT-3");
                break;
            }
            case 4: {
                addTimeZone("PKT  Pakistan", "PKT-5");
                addTimeZone("IST  India", "IST-5:30");
                addTimeZone("WIB  Western Indonesia", "WIB-7");
                addTimeZone("WITA Central Indonesia", "WITA-8");
                addTimeZone("WIT  Eastern Indonesia", "WIT-9");
                addTimeZone("CST  China", "CST-8");
                addTimeZone("HKT  Hong Kong", "HKT-8");
                addTimeZone("PST  Philippines", "PST-8");
                addTimeZone("JST  Japan", "JST-9");
                addTimeZone("KST  Korea", "KST-9");
                break;
            }
            case 5: {
                addTimeZone("AWST         Western Australia", "AWST-8");
                addTimeZone("ACST         Central Australia", "ACST-9:30");
                addTimeZone("ACST/ACDT    Central Australia", "ACST-9:30ACDT,M10.1.0,M4.1.0/3");
                addTimeZone("AEST         Eastern Australia", "AEST-10");
                addTimeZone("AEST/AEDT    Eastern Australia", "AEST-10AEDT,M10.1.0,M4.1.0/3");
                addTimeZone("NZST/NZDT    New Zealand", "NZST-12NZDT,M9.5.0,M4.1.0/3");
                addTimeZone("CHAST/CHADT  Chatham Island", "CHAST-12:45CHADT,M9.5.0/2:45,M4.1.0/3:45");
                break;
            }
        }
    }
};
