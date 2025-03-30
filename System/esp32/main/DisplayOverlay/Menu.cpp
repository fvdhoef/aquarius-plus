#include "Menu.h"
#include "DisplayOverlay.h"
#include "Keyboard.h"

void Menu::draw() {
    auto ovl = getDisplayOverlay();
    ovl->clearScreen();

    int numRows     = (int)items.size();
    int x           = (40 - width) / 2;
    int height      = getHeight();
    int y           = (25 - height) / 2;
    int visibleRows = getVisibleRows();

    {
        if (firstRow > selectedRow - 3) {
            firstRow = std::max(0, selectedRow - 3);
        }
        if (selectedRow - firstRow >= visibleRows - 3) {
            firstRow = selectedRow - visibleRows + 1 + 3;
        }
        if (firstRow + visibleRows > numRows) {
            firstRow = numRows - visibleRows;
        }
    }

    {
        int selRow = selectedRow;
        if (selRow >= 0 && items[selRow].type == MenuItemType::separator)
            selRow = -1;
        else {
            selRow -= firstRow;
        }

        ovl->drawBorder(
            x, y, width, height,
            colBorder, colBg,
            (selRow < 0) ? -1 : selRow + (title.empty() ? 0 : 2),
            colSelectedBg);
    }

    auto attrSelected = DisplayOverlay::makeAttr(colSelectedFg, colSelectedBg);
    auto attrNormal   = DisplayOverlay::makeAttr(colSelectedBg, colBg);

    y++;

    if (!title.empty()) {
        int x2 = x + 1;
        if (!isRootMenu) {
            ovl->drawStr(x2, y, DisplayOverlay::makeAttr(colTitleFg, colBg), "<");
            x2 += 2;
        }
        ovl->drawStr(x2, y, DisplayOverlay::makeAttr(colTitleFg, colBg), title.c_str());
        y += 2;
    }

    for (int i = firstRow, j = 0; i < numRows && j < visibleRows; i++, j++, y++) {
        auto attr = (selectedRow == i) ? attrSelected : attrNormal;

        auto &mi = items[i];
        if (mi.type == MenuItemType::separator && mi.name.empty())
            continue;

        char str[40];
        int  len = snprintf(str, sizeof(str), "%s", mi.name.c_str());
        while (len < width - 2) {
            str[len++] = ' ';
        }
        str[len] = 0;

        switch (mi.type) {
            case MenuItemType::separator: {
                attr = DisplayOverlay::makeAttr(colSeparatorFg, colBg);
                break;
            }

            case MenuItemType::percentage: {
                if (mi.getter)
                    snprintf(str + len - 3, 4, "%3d", mi.getter());
                break;
            }

            case MenuItemType::onOff: {
                if (mi.getter)
                    snprintf(str + len - 3, 4, "%3s", mi.getter() ? "on" : "off");
                break;
            }

            case MenuItemType::subMenu: {
                if (mi.onEnter)
                    str[len - 1] = '>';
                break;
            }

            default: break;
        }

        ovl->drawStr(x + 1, y, attr, str);
    }

    ovl->render();
}

enum {
    CH_UP        = 0x8F,
    CH_DOWN      = 0x9F,
    CH_LEFT      = 0x9E,
    CH_RIGHT     = 0x8E,
    CH_HOME      = 0x9B,
    CH_END       = 0x9A,
    CH_DELETE    = 0x7F,
    CH_PGUP      = 0x8A,
    CH_PGDN      = 0x8B,
    CH_ENTER     = '\r',
    CH_BACKSPACE = '\b',
    CH_ESC       = 3,
};

void Menu::show() {
    auto keyboard = Keyboard::instance();
    auto ovl      = getDisplayOverlay();

    auto prevTicks = xTaskGetTickCount();
    onEnter();

    needsUpdate = true;
    needsRedraw = true;
    selectedRow = 0;
    firstRow    = 0;

    while (!exitMenu && !getDisplayOverlay()->shouldReinit()) {
        bool ovlVisible = ovl->isVisible();

        auto curTicks = xTaskGetTickCount();
        if (curTicks - prevTicks > pdMS_TO_TICKS(1000)) {
            if (ovlVisible)
                needsRedraw |= onTick();
            prevTicks = curTicks;
        }

        if (needsUpdate) {
            onUpdate();
            needsUpdate = false;
        }

        // Make sure selectedRow is valid
        {
            selectedRow = items.empty() ? -1 : (std::min(std::max(selectedRow, 0), (int)items.size() - 1));

            while (selectedRow < (int)items.size() - 1 && items[selectedRow].type == MenuItemType::separator) {
                selectedRow++;
            }
        }

        if (needsRedraw) {
            draw();
            needsRedraw = false;
        }

        int ch = keyboard->getKey(pdMS_TO_TICKS(100));
        if (ch < 0)
            continue;

        if (ch == 0xFF) {
            ovlVisible = !ovlVisible;
            ovl->setVisible(ovlVisible);
            continue;
        }
        if (!ovlVisible)
            continue;

        if (ch == CH_ESC) {
            // Exit menu
            break;
        }

        needsRedraw = true;

        if (!items.empty()) {
            switch (ch) {
                case CH_UP: {
                    do {
                        selectedRow--;
                    } while (selectedRow > 0 && items[selectedRow].type == MenuItemType::separator);
                    break;
                }

                case CH_DOWN: {
                    do {
                        selectedRow++;
                    } while (selectedRow < (int)items.size() - 1 && items[selectedRow].type == MenuItemType::separator);
                    break;
                }

                case CH_LEFT: {
                    auto &mi = items[selectedRow];
                    switch (mi.type) {
                        case MenuItemType::percentage: {
                            if (mi.setter && mi.getter)
                                mi.setter(std::min(std::max(mi.getter() - 1, 0), 100));
                            break;
                        }
                        case MenuItemType::onOff: {
                            if (mi.getter && mi.setter && mi.getter() != 0)
                                mi.setter(false);
                            break;
                        }
                        default: break;
                    }
                    break;
                }

                case CH_RIGHT: {
                    auto &mi = items[selectedRow];
                    switch (mi.type) {
                        case MenuItemType::percentage: {
                            if (mi.setter && mi.getter)
                                mi.setter(std::min(std::max(mi.getter() + 1, 0), 100));
                            break;
                        }
                        case MenuItemType::onOff: {
                            if (mi.getter && mi.setter && mi.getter() == 0)
                                mi.setter(true);
                            break;
                        }
                        default: break;
                    }
                    break;
                }

                case CH_PGUP: {
                    selectedRow = std::max(0, selectedRow - getVisibleRows());
                    break;
                }

                case CH_PGDN: {
                    selectedRow = std::min((int)items.size() - 1, selectedRow + getVisibleRows());
                    break;
                }

                case CH_ENTER: {
                    auto &mi = items[selectedRow];
                    // printf("Enter on row:%d '%s' has_onEnter:%d\n", selectedRow, mi.name.c_str(), (bool)mi.onEnter);

                    if (mi.onEnter) {
                        mi.onEnter();
                    } else if (mi.type == MenuItemType::onOff) {
                        if (mi.getter && mi.setter)
                            mi.setter(mi.getter() ? 0 : 1);
                    }
                    break;
                }
            }
        }
    }
    exitMenu = false;

    onExit();
}

void Menu::drawMessage(const char *msg) {
    auto ovl = getDisplayOverlay();
    ovl->clearScreen();

    int width  = strlen(msg) + 2;
    int height = 3;

    int x = (40 - width) / 2;
    int y = (25 - height) / 2;

    ovl->drawBorder(x, y, width, height, colBorder, colBg);
    ovl->drawStr(x + 1, y + 1, DisplayOverlay::makeAttr(colFg, colBg), msg);

    ovl->render();
}

bool Menu::editString(const std::string &title, std::string &str, int maxLen, bool isPassword) {
    auto keyboard = Keyboard::instance();
    auto ovl      = getDisplayOverlay();

    int width  = 38;
    int height = 5;

    int cursor        = (int)str.size();
    int firstIdx      = 0;
    int maxWidthShown = width - 2;

    while (1) {
        {
            int strSize = (int)str.size();
            cursor      = std::max(0, std::min(cursor, strSize));
            if (firstIdx > cursor - 3) {
                firstIdx = std::max(0, cursor - 3);
            }
            if (cursor - firstIdx > maxWidthShown) {
                firstIdx = cursor - maxWidthShown;
            }
            if (strSize < maxWidthShown) {
                firstIdx = 0;
            }
            if (cursor - firstIdx >= maxWidthShown)
                firstIdx = std::max(0, cursor - maxWidthShown + 1);
        }

        ovl->clearScreen();

        int x = (40 - width) / 2;
        int y = (25 - height) / 2;

        ovl->drawBorder(x, y, width, height, colBorder, colBg);
        ovl->drawStr(x + 1, y + 1, DisplayOverlay::makeAttr(colTitleFg, colBg), title.c_str());
        ovl->fill(x + 1, y + 3, width - 2, 1, DisplayOverlay::makeAttr(colSelectedFg, colSelectedBg));
        {
            auto subStr = str.substr(firstIdx, maxWidthShown);
            if (isPassword) {
                for (auto &ch : subStr)
                    ch = '*';
            }
            ovl->drawStr(x + 1, y + 3, DisplayOverlay::makeAttr(colSelectedFg, colSelectedBg), subStr.c_str());
        }
        ovl->setAttr(x + 1 + cursor - firstIdx, y + 3, DisplayOverlay::makeAttr(colSelectedBg, colSelectedFg));
        ovl->render();

        int ch = keyboard->getKey(portMAX_DELAY);
        switch (ch) {
            case CH_ESC: return false;
            case CH_ENTER: return true;

            case CH_DELETE: {
                if (cursor < (int)str.size()) {
                    str.erase(str.begin() + cursor);
                }
                break;
            }

            case CH_BACKSPACE: {
                if (cursor > 0) {
                    cursor--;
                    str.erase(str.begin() + cursor);
                }
                break;
            }

            case CH_LEFT: {
                cursor--;
                break;
            }
            case CH_RIGHT: {
                cursor++;
                break;
            }
            case CH_HOME: {
                cursor = 0;
                break;
            }
            case CH_END: {
                cursor = (int)str.size();
                break;
            }

            default: {
                if (ch >= ' ' && ch <= '~') {
                    if (maxLen >= 0 && (int)str.size() < maxLen) {
                        str.insert(str.begin() + cursor, ch);
                        cursor++;
                    }
                }
                break;
            }
        }
    }
}
