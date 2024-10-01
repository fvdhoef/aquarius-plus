#include "Menu.h"
#include "DisplayOverlay.h"
#include "Keyboard.h"

void Menu::draw(int selectedRow) {
    auto ovl = getDisplayOverlay();
    ovl->clearScreen();

    int x      = (40 - width) / 2;
    int height = 1 + (title.empty() ? 0 : 2) + items.size() + 1;
    int y      = (25 - height) / 2;

    {
        int selRow = selectedRow;
        if (selRow >= 0 && items[selRow].type == MenuItemType::separator)
            selRow = -1;

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
        ovl->drawStr(x + 1, y, DisplayOverlay::makeAttr(colTitleFg, colBg), title.c_str());
        y += 2;
    }

    for (int i = 0; i < (int)items.size(); i++) {
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

        ovl->drawStr(x + 1, y + i, attr, str);
    }

    ovl->render();
}

enum {
    CH_UP        = 143,
    CH_DOWN      = 159,
    CH_LEFT      = 158,
    CH_RIGHT     = 142,
    CH_ENTER     = '\r',
    CH_BACKSPACE = '\b',
    CH_ESC       = 3,
};

void Menu::show() {
    auto keyboard = getKeyboard();
    auto ovl      = getDisplayOverlay();

    auto prevTicks = xTaskGetTickCount();
    onEnter();

    needsUpdate = true;
    needsRedraw = true;
    selectedRow = 0;

    while (!exitMenu) {
        bool ovlVisible = ovl->isVisible();

        auto curTicks = xTaskGetTickCount();
        if (curTicks - prevTicks > pdMS_TO_TICKS(1000)) {
            if (ovlVisible)
                needsRedraw |= onTick();
            prevTicks = curTicks;
        }

        if (needsUpdate) {
            onUpdate();
        }

        selectedRow = items.empty() ? -1 : (std::min(std::max(selectedRow, 0), (int)items.size() - 1));

        if (needsRedraw) {
            draw(selectedRow);
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

                case CH_ENTER: {
                    auto &mi = items[selectedRow];
                    // printf("Enter on row:%d '%s' has_onEnter:%d\n", selectedRow, mi.name.c_str(), (bool)mi.onEnter);

                    if (mi.onEnter)
                        mi.onEnter();
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

bool Menu::editString(const std::string &title, std::string &value) {
    auto keyboard = getKeyboard();
    auto ovl      = getDisplayOverlay();

    while (1) {
        ovl->clearScreen();

        int width  = 38;
        int height = 5;

        int x = (40 - width) / 2;
        int y = (25 - height) / 2;

        ovl->drawBorder(x, y, width, height, colBorder, colBg);
        ovl->drawStr(x + 1, y + 1, DisplayOverlay::makeAttr(colTitleFg, colBg), title.c_str());
        ovl->drawStr(x + 1, y + 3, DisplayOverlay::makeAttr(colSelectedFg, colSelectedBg), value.c_str());
        ovl->render();

        int ch = keyboard->getKey(portMAX_DELAY);
        switch (ch) {
            case CH_ESC: return false;
            case CH_ENTER: return true;

            case CH_BACKSPACE: {
                if (!value.empty())
                    value.pop_back();
                break;
            }

            default: {
                if (ch >= ' ' && ch <= '~')
                    value.push_back(ch);
                break;
            }
        }
    }
}
