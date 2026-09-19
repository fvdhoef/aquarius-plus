#include "KbHcEmu.h"

#define _USE_MATH_DEFINES

#include "AqKeyboardDefs.h"
#include "DisplayOverlay/FileListMenu.h"
#include "Keyboard.h"
#include "UartProtocol.h"
#include "VFS.h"

#include <math.h>
#include <nvs_flash.h>

KbHcEmu::KbHcEmu() {
    memset(gamePads, 0, sizeof(gamePads));
}

void KbHcEmu::loadSettings() {
    nvs_handle_t h;
    if (nvs_open("settings", NVS_READONLY, &h) == ESP_OK) {
        uint8_t val8 = 0;
        if (nvs_get_u8(h, "gamepadNav", &val8) == ESP_OK) {
            gamepadNavigation = val8 != 0;
        }

        size_t sz;
        sz = sizeof(kb2hcSettings);
        if (nvs_get_blob(h, "kb2hc", &kb2hcSettings, &sz) != ESP_OK || sz != sizeof(kb2hcSettings)) {
            kb2hcSettings = Kb2HcMapping();
        }
        sz = sizeof(gp2hcSettings);
        if (nvs_get_blob(h, "gp2hc", &gp2hcSettings, &sz) != ESP_OK || sz != sizeof(gp2hcSettings)) {
            gp2hcSettings = Gp2HcMapping();
        }
        sz = sizeof(gp2kbSettings);
        if (nvs_get_blob(h, "gp2kb", &gp2kbSettings, &sz) != ESP_OK || sz != sizeof(gp2kbSettings)) {
            gp2kbSettings = Gp2KbMapping();
        }
        nvs_close(h);
    }
}

bool KbHcEmu::handControllerEmulate(unsigned scanCode, bool keyDown) {
    keybHandCtrl1 = 0xFF;
    if (!kb2hcSettings.enabled)
        return false;

    bool result = true;

    enum {
        UP    = (1 << 0),
        DOWN  = (1 << 1),
        LEFT  = (1 << 2),
        RIGHT = (1 << 3),
        K1    = (1 << 4),
        K2    = (1 << 5),
        K3    = (1 << 6),
        K4    = (1 << 7),
        K5    = (1 << 8),
        K6    = (1 << 9),
    };

    if (scanCode == SCANCODE_UP)
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | UP) : (keybHandCtrl1Pressed & ~UP);
    else if (scanCode == SCANCODE_DOWN)
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | DOWN) : (keybHandCtrl1Pressed & ~DOWN);
    else if (scanCode == SCANCODE_LEFT)
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | LEFT) : (keybHandCtrl1Pressed & ~LEFT);
    else if (scanCode == SCANCODE_RIGHT)
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | RIGHT) : (keybHandCtrl1Pressed & ~RIGHT);
    else if (scanCode == kb2hcSettings.buttonScanCodes[0])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K1) : (keybHandCtrl1Pressed & ~K1);
    else if (scanCode == kb2hcSettings.buttonScanCodes[1])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K2) : (keybHandCtrl1Pressed & ~K2);
    else if (scanCode == kb2hcSettings.buttonScanCodes[2])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K3) : (keybHandCtrl1Pressed & ~K3);
    else if (scanCode == kb2hcSettings.buttonScanCodes[3])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K4) : (keybHandCtrl1Pressed & ~K4);
    else if (scanCode == kb2hcSettings.buttonScanCodes[4])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K5) : (keybHandCtrl1Pressed & ~K5);
    else if (scanCode == kb2hcSettings.buttonScanCodes[5])
        keybHandCtrl1Pressed = keyDown ? (keybHandCtrl1Pressed | K6) : (keybHandCtrl1Pressed & ~K6);
    else
        result = false;

    switch (keybHandCtrl1Pressed & 0xF) {
        case LEFT: keybHandCtrl1 &= ~(1 << 3); break;
        case UP | LEFT: keybHandCtrl1 &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case UP: keybHandCtrl1 &= ~(1 << 2); break;
        case UP | RIGHT: keybHandCtrl1 &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case RIGHT: keybHandCtrl1 &= ~(1 << 1); break;
        case DOWN | RIGHT: keybHandCtrl1 &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case DOWN: keybHandCtrl1 &= ~(1 << 0); break;
        case DOWN | LEFT: keybHandCtrl1 &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        default: break;
    }
    if (keybHandCtrl1Pressed & K1)
        keybHandCtrl1 &= ~(1 << 6);
    if (keybHandCtrl1Pressed & K2)
        keybHandCtrl1 &= ~((1 << 7) | (1 << 2));
    if (keybHandCtrl1Pressed & K3)
        keybHandCtrl1 &= ~((1 << 7) | (1 << 5));
    if (keybHandCtrl1Pressed & K4)
        keybHandCtrl1 &= ~(1 << 5);
    if (keybHandCtrl1Pressed & K5)
        keybHandCtrl1 &= ~((1 << 7) | (1 << 1));
    if (keybHandCtrl1Pressed & K6)
        keybHandCtrl1 &= ~((1 << 7) | (1 << 0));

    return result;
}

bool KbHcEmu::keyScancode(uint8_t modifiers, unsigned scanCode, bool keyDown) {
    // Hand controller emulation
    if (handControllerEmulate(scanCode, keyDown)) {
        if (updateHandCtrl)
            updateHandCtrl(gamePadHandCtrl[0] & keybHandCtrl1, gamePadHandCtrl[1]);
        return true;
    }

    // Keyboard matrix emulation
    {
        int key = -1;
        switch (scanCode) {
            case SCANCODE_EQUALS: key = KEY_EQUALS; break;
            case SCANCODE_BACKSPACE: key = KEY_BACKSPACE; break;
            case SCANCODE_APOSTROPHE: key = KEY_COLON; break;
            case SCANCODE_RETURN: key = KEY_RETURN; break;
            case SCANCODE_SEMICOLON: key = KEY_SEMICOLON; break;
            case SCANCODE_PERIOD: key = KEY_PERIOD; break;
            case SCANCODE_INSERT: key = KEY_INSERT; break;
            case SCANCODE_DELETE: key = KEY_DELETE; break;
            case SCANCODE_MINUS: key = KEY_MINUS; break;
            case SCANCODE_SLASH: key = KEY_SLASH; break;
            case SCANCODE_0: key = KEY_0; break;
            case SCANCODE_P: key = KEY_P; break;
            case SCANCODE_L: key = KEY_L; break;
            case SCANCODE_COMMA: key = KEY_COMMA; break;
            case SCANCODE_UP: key = KEY_UP; break;
            case SCANCODE_RIGHT: key = KEY_RIGHT; break;
            case SCANCODE_9: key = KEY_9; break;
            case SCANCODE_O: key = KEY_O; break;
            case SCANCODE_K: key = KEY_K; break;
            case SCANCODE_M: key = KEY_M; break;
            case SCANCODE_N: key = KEY_N; break;
            case SCANCODE_J: key = KEY_J; break;
            case SCANCODE_LEFT: key = KEY_LEFT; break;
            case SCANCODE_DOWN: key = KEY_DOWN; break;
            case SCANCODE_8: key = KEY_8; break;
            case SCANCODE_I: key = KEY_I; break;
            case SCANCODE_7: key = KEY_7; break;
            case SCANCODE_U: key = KEY_U; break;
            case SCANCODE_H: key = KEY_H; break;
            case SCANCODE_B: key = KEY_B; break;
            case SCANCODE_HOME: key = KEY_HOME; break;
            case SCANCODE_END: key = KEY_END; break;
            case SCANCODE_6: key = KEY_6; break;
            case SCANCODE_Y: key = KEY_Y; break;
            case SCANCODE_G: key = KEY_G; break;
            case SCANCODE_V: key = KEY_V; break;
            case SCANCODE_C: key = KEY_C; break;
            case SCANCODE_F: key = KEY_F; break;
            case SCANCODE_PAGEUP: key = KEY_PGUP; break;
            case SCANCODE_PAGEDOWN: key = KEY_PGDN; break;
            case SCANCODE_5: key = KEY_5; break;
            case SCANCODE_T: key = KEY_T; break;
            case SCANCODE_4: key = KEY_4; break;
            case SCANCODE_R: key = KEY_R; break;
            case SCANCODE_D: key = KEY_D; break;
            case SCANCODE_X: key = KEY_X; break;
            case SCANCODE_PAUSE: key = KEY_PAUSE; break;
            case SCANCODE_PRINTSCREEN: key = KEY_PRTSCR; break;
            case SCANCODE_3: key = KEY_3; break;
            case SCANCODE_E: key = KEY_E; break;
            case SCANCODE_S: key = KEY_S; break;
            case SCANCODE_Z: key = KEY_Z; break;
            case SCANCODE_SPACE: key = KEY_SPACE; break;
            case SCANCODE_A: key = KEY_A; break;
            case SCANCODE_APPLICATION: key = KEY_MENU; break;
            case SCANCODE_TAB: key = KEY_TAB; break;
            case SCANCODE_2: key = KEY_2; break;
            case SCANCODE_W: key = KEY_W; break;
            case SCANCODE_1: key = KEY_1; break;
            case SCANCODE_Q: key = KEY_Q; break;
            default: break;
        }
        if (key >= 0) {
            if (keyDown)
                keybMatrix |= (1ULL << key);
            else
                keybMatrix &= ~(1ULL << key);
        }

        if (modifiers & (ModLShift | ModRShift))
            keybMatrix |= (1ULL << KEY_SHIFT);
        else
            keybMatrix &= ~(1ULL << KEY_SHIFT);

        if (modifiers & (ModLAlt | ModRAlt))
            keybMatrix |= (1ULL << KEY_ALT);
        else
            keybMatrix &= ~(1ULL << KEY_ALT);

        if (modifiers & (ModLCtrl | ModRCtrl))
            keybMatrix |= (1ULL << KEY_CTRL);
        else
            keybMatrix &= ~(1ULL << KEY_CTRL);

        if (modifiers & (ModLGui | ModRGui))
            keybMatrix |= (1ULL << KEY_GUI);
        else
            keybMatrix &= ~(1ULL << KEY_GUI);

        // Handle ESCAPE as if CTRL-C is pressed
        if (scanCode == SCANCODE_ESCAPE) {
            if (keyDown) {
                keybMatrix |= (1ULL << KEY_C) | (1ULL << KEY_CTRL);
            } else {
                keybMatrix &= ~((1ULL << KEY_C) | (1ULL << KEY_CTRL));
            }
        }
    }

    if (prevMatrix != keybMatrix) {
        // printf("keybMatrix: %016llx\n", keybMatrix);
        if (updateKeybMatrix)
            updateKeybMatrix(~keybMatrix);
        prevMatrix = keybMatrix;
    }
    return false;
}

void KbHcEmu::gamepadReport(unsigned idx, const GamePadData &data) {
    if (idx > 1)
        return;

    uint16_t pressed = (~gamePads[idx].buttons & data.buttons);
    uint16_t changed = (gamePads[idx].buttons ^ data.buttons);

    // printf("idx=%u pressed=%04X\n", idx, pressed);

    if (idx == 0) {
        bool overlayVisible = getDisplayOverlay()->isVisible();
        auto kb             = Keyboard::instance();

        if (gamepadNavigation) {
            if (pressed & GCB_GUIDE) {
                kb->handleScancode(SCANCODE_LCTRL, true);
                kb->handleScancode(SCANCODE_TAB, true);
                kb->handleScancode(SCANCODE_TAB, false);
                kb->handleScancode(SCANCODE_LCTRL, false);
            }

            if (overlayVisible) {
                if (changed & GCB_DPAD_UP)
                    kb->handleScancode(SCANCODE_UP, (data.buttons & GCB_DPAD_UP) != 0);
                if (changed & GCB_DPAD_DOWN)
                    kb->handleScancode(SCANCODE_DOWN, (data.buttons & GCB_DPAD_DOWN) != 0);
                if (changed & GCB_DPAD_LEFT)
                    kb->handleScancode(SCANCODE_LEFT, (data.buttons & GCB_DPAD_LEFT) != 0);
                if (changed & GCB_DPAD_RIGHT)
                    kb->handleScancode(SCANCODE_RIGHT, (data.buttons & GCB_DPAD_RIGHT) != 0);
                if (changed & GCB_A)
                    kb->handleScancode(SCANCODE_RETURN, (data.buttons & GCB_A) != 0);
                if (changed & GCB_B)
                    kb->handleScancode(SCANCODE_ESCAPE, (data.buttons & GCB_B) != 0);
            }
        }

        if (!overlayVisible && gp2kbSettings.enabled) {
            for (int i = 0; i < 16; i++) {
                if (gp2kbSettings.buttonScanCodes[i] != 0 && changed & (1 << i))
                    kb->handleScancode(gp2kbSettings.buttonScanCodes[i], (data.buttons & (1 << i)) != 0);
            }
        }
    }

    if (memcmp(&gamePads[idx], &data, sizeof(data)) != 0) {
        gamePads[idx] = data;
        if (updateGamePad)
            updateGamePad(idx, data);
    }

    gameCtrlUpdated();
}

void KbHcEmu::gameCtrlUpdated() {
    gamePadHandCtrl[0] = 0xFF;
    gamePadHandCtrl[1] = 0xFF;

    if (!gp2hcSettings.enabled)
        return;

    static const uint8_t buttonMasks[] = {
        (1 << 6),            // 1
        (1 << 7) | (1 << 2), // 2
        (1 << 7) | (1 << 5), // 3
        (1 << 5),            // 4
        (1 << 7) | (1 << 1), // 5
        (1 << 7) | (1 << 0), // 6
    };

    for (int i = 0; i < 2; i++) {
        for (int btnIdx = 0; btnIdx < 16; btnIdx++) {
            if ((gamePads[i].buttons & (1 << btnIdx)) == 0)
                continue;

            uint8_t button = gp2hcSettings.buttonNumber[btnIdx];
            if (button >= 1 && button <= 6)
                gamePadHandCtrl[i] &= ~buttonMasks[button - 1];
        }

        // Map D-pad on hand controller disc
        unsigned p = 0;
        if ((gamePads[i].buttons & GCB_DPAD_UP) == GCB_DPAD_UP)
            p = 13;
        else if ((gamePads[i].buttons & (GCB_DPAD_UP | GCB_DPAD_RIGHT)) == (GCB_DPAD_UP | GCB_DPAD_RIGHT))
            p = 15;
        else if ((gamePads[i].buttons & GCB_DPAD_RIGHT) == GCB_DPAD_RIGHT)
            p = 1;
        else if ((gamePads[i].buttons & (GCB_DPAD_DOWN | GCB_DPAD_RIGHT)) == (GCB_DPAD_DOWN | GCB_DPAD_RIGHT))
            p = 3;
        else if ((gamePads[i].buttons & GCB_DPAD_DOWN) == GCB_DPAD_DOWN)
            p = 5;
        else if ((gamePads[i].buttons & (GCB_DPAD_DOWN | GCB_DPAD_LEFT)) == (GCB_DPAD_DOWN | GCB_DPAD_LEFT))
            p = 7;
        else if ((gamePads[i].buttons & GCB_DPAD_LEFT) == GCB_DPAD_LEFT)
            p = 9;
        else if ((gamePads[i].buttons & (GCB_DPAD_UP | GCB_DPAD_LEFT)) == (GCB_DPAD_UP | GCB_DPAD_LEFT))
            p = 11;

        {
            float x = gamePads[i].lx / 128.0f;
            float y = gamePads[i].ly / 128.0f;

            float len   = sqrtf(x * x + y * y);
            float angle = 0;
            if (len > 0.4f) {
                angle = atan2f(y, x) / (float)M_PI * 180.0f + 180.0f;
                p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
            }
        }

        switch (p) {
            case 1: gamePadHandCtrl[i] &= ~((1 << 1)); break;
            case 2: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 1)); break;
            case 3: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
            case 4: gamePadHandCtrl[i] &= ~((1 << 1) | (1 << 0)); break;
            case 5: gamePadHandCtrl[i] &= ~((1 << 0)); break;
            case 6: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 0)); break;
            case 7: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
            case 8: gamePadHandCtrl[i] &= ~((1 << 3) | (1 << 0)); break;
            case 9: gamePadHandCtrl[i] &= ~((1 << 3)); break;
            case 10: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 3)); break;
            case 11: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
            case 12: gamePadHandCtrl[i] &= ~((1 << 3) | (1 << 2)); break;
            case 13: gamePadHandCtrl[i] &= ~((1 << 2)); break;
            case 14: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 2)); break;
            case 15: gamePadHandCtrl[i] &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
            case 16: gamePadHandCtrl[i] &= ~((1 << 2) | (1 << 1)); break;
            default: break;
        }
    }

    if (updateHandCtrl)
        updateHandCtrl(gamePadHandCtrl[0] & keybHandCtrl1, gamePadHandCtrl[1]);
}

bool KbHcEmu::getGamePadData(unsigned idx, GamePadData &data) {
    if (idx > 1)
        return false;

    data = gamePads[idx];
    return true;
}

void KbHcEmu::cmdGetGameCtrl(uint8_t idx) {
    // DBGF("GETGAMECTRL");

    auto up = UartProtocol::instance();
    up->txStart();
    if (idx > 1) {
        up->txWrite(ERR_NOT_FOUND);
        return;
    }
    up->txWrite(0);
    up->txWrite(gamePads[idx].lx);
    up->txWrite(gamePads[idx].ly);
    up->txWrite(gamePads[idx].rx);
    up->txWrite(gamePads[idx].ry);
    up->txWrite(gamePads[idx].lt);
    up->txWrite(gamePads[idx].rt);
    up->txWrite(gamePads[idx].buttons & 0xFF);
    up->txWrite(gamePads[idx].buttons >> 8);
}

std::string KbHcEmu::getPresetPath(std::string presetType) {
    return std::string("/config/esp32/") + coreName + "/" + presetType;
}

void KbHcEmu::savePreset(Menu &menu, std::string presetType, const void *buf, size_t size) {
    std::string presetName;
    if (menu.editString("Enter preset name", presetName, 32)) {
        presetName = trim(presetName, " \t\n\r\f\v/\\");

        if (!presetName.empty()) {
            auto vfs = getSDCardVFS();

            std::string path = getPresetPath(presetType);
            if (createPath(path)) {
                path += "/" + presetName;

                int fd = vfs->open(FO_WRONLY | FO_CREATE, path);
                if (fd >= 0) {
                    vfs->write(fd, size, buf);
                    vfs->close(fd);
                }
            }
        }
    }
}
void KbHcEmu::loadPreset(std::string presetType, void *buf, size_t size) {
    FileListMenu menu;
    menu.title    = "Select preset";
    menu.path     = getPresetPath(presetType);
    menu.onSelect = [buf, size](const std::string &path) {
        auto vfs = getSDCardVFS();
        int  fd  = vfs->open(FO_RDONLY, path);
        if (fd >= 0) {
            vfs->read(fd, size, buf);
            vfs->close(fd);
        }
    };
    menu.show();
}

void KbHcEmu::addMainMenuItems(Menu &menu) {
    {
        auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Keyboard to hand ctrl mapping");
        item.onEnter = [this]() {
            KeyboardHandCtrlMappingMenu menu;
            menu.settings = kb2hcSettings;
            menu.onChange = [this, &menu]() {
                kb2hcSettings = menu.settings;

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_blob(h, "kb2hc", &kb2hcSettings, sizeof(kb2hcSettings)) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            };
            menu.onSave = [this, &menu]() { savePreset(menu, "map_kb_hc", menu.settings.buttonScanCodes, sizeof(menu.settings.buttonScanCodes)); };
            menu.onLoad = [this, &menu]() { loadPreset("map_kb_hc", menu.settings.buttonScanCodes, sizeof(menu.settings.buttonScanCodes)); menu.onChange(); };
            menu.show();
        };
    }
    {
        auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Gamepad to hand ctrl mapping");
        item.onEnter = [this]() {
            GamepadHandCtrlMappingMenu menu;
            menu.settings = gp2hcSettings;
            menu.onChange = [this, &menu]() {
                gp2hcSettings = menu.settings;

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_blob(h, "gp2hc", &gp2hcSettings, sizeof(gp2hcSettings)) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            };
            menu.onSave = [this, &menu]() { savePreset(menu, "map_gp_hc", menu.settings.buttonNumber, sizeof(menu.settings.buttonNumber)); };
            menu.onLoad = [this, &menu]() { loadPreset("map_gp_hc", menu.settings.buttonNumber, sizeof(menu.settings.buttonNumber)); menu.onChange(); };
            menu.show();
        };
    }
    {
        auto &item   = menu.items.emplace_back(MenuItemType::subMenu, "Gamepad to keyboard mapping");
        item.onEnter = [this]() {
            GamepadKeyboardMappingMenu menu;
            menu.settings = gp2kbSettings;
            menu.onChange = [this, &menu]() {
                gp2kbSettings = menu.settings;

                nvs_handle_t h;
                if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                    if (nvs_set_blob(h, "gp2kb", &gp2kbSettings, sizeof(gp2kbSettings)) == ESP_OK) {
                        nvs_commit(h);
                    }
                    nvs_close(h);
                }
            };
            menu.onSave = [this, &menu]() { savePreset(menu, "map_gp_kb", menu.settings.buttonScanCodes, sizeof(menu.settings.buttonScanCodes)); };
            menu.onLoad = [this, &menu]() { loadPreset("map_gp_kb", menu.settings.buttonScanCodes, sizeof(menu.settings.buttonScanCodes)); menu.onChange(); };
            menu.show();
        };
    }
    {
        auto &item  = menu.items.emplace_back(MenuItemType::onOff, "Navigate menu using gamepad");
        item.setter = [this](int newVal) {
            gamepadNavigation = (newVal != 0);

            nvs_handle_t h;
            if (nvs_open("settings", NVS_READWRITE, &h) == ESP_OK) {
                if (nvs_set_u8(h, "gamepadNav", gamepadNavigation ? 1 : 0) == ESP_OK) {
                    nvs_commit(h);
                }
                nvs_close(h);
            }
        };
        item.getter = [this]() { return gamepadNavigation ? 1 : 0; };
    }
}
