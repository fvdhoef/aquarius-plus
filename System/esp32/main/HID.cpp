#include "HID.h"
#include "FPGA.h"
#include "AqKeyboard.h"
#include <math.h>

static const char *TAG = "hid";

#pragma pack(push, 1)
struct xbox_hid_data {
    uint16_t lx;
    uint16_t ly;
    uint16_t rx;
    uint16_t ry;
    uint16_t lt;
    uint16_t rt;
    uint8_t  dpad;

    unsigned btn_a : 1;
    unsigned btn_b : 1;
    unsigned : 1;
    unsigned btn_x : 1;
    unsigned btn_y : 1;
    unsigned : 1;
    unsigned btn_lb : 1;
    unsigned btn_rb : 1;

    unsigned : 2;
    unsigned btn_view : 1;
    unsigned btn_menu : 1;
    unsigned btn_xbox : 1;
    unsigned btn_l3 : 1;
    unsigned btn_r3 : 1;
    unsigned : 1;

    unsigned btn_share : 1;
    unsigned : 7;
};
#pragma pack(pop)

void handleXboxData(const uint8_t data[16]) {
    struct xbox_hid_data hd;
    memcpy(&hd, data, sizeof(hd));
    // printf("%d\n", sizeof(hd));

    // ESP_LOGI(
    //     TAG, "lx:%6d ly:%6d rx:%6d ru:%6d lt:%4u rt:%4u dpad:%u A:%u B:%u X:%u Y:%u LB:%u RB:%u VIEW:%u MENU:%u XBOX:%u L3:%u R3:%u SHARE:%u",
    //     (int)hd.lx - 0x8000, (int)hd.ly - 0x8000,
    //     (int)hd.rx - 0x8000, (int)hd.ry - 0x8000,
    //     hd.lt, hd.rt, hd.dpad,
    //     hd.btn_a, hd.btn_b, hd.btn_x, hd.btn_y, hd.btn_lb, hd.btn_rb,
    //     hd.btn_view, hd.btn_menu, hd.btn_xbox, hd.btn_l3, hd.btn_r3,
    //     hd.btn_share);

    uint8_t handctrl = 0xFF;
    if (hd.btn_a)
        handctrl &= ~(1 << 6);
    if (hd.btn_b)
        handctrl &= ~((1 << 7) | (1 << 2));
    if (hd.btn_x)
        handctrl &= ~((1 << 7) | (1 << 5));
    if (hd.btn_y)
        handctrl &= ~((1 << 5));
    if (hd.btn_lb)
        handctrl &= ~((1 << 7) | (1 << 1));
    if (hd.btn_rb)
        handctrl &= ~((1 << 7) | (1 << 0));

    unsigned p = 0;
    switch (hd.dpad) {
        case 1: p = 13; break; // UP
        case 2: p = 15; break; // UP+RIGHT
        case 3: p = 1; break;  // RIGHT
        case 4: p = 3; break;  // DOWN+RIGHT
        case 5: p = 5; break;  // DOWN
        case 6: p = 7; break;  // DOWN+LEFT
        case 7: p = 9; break;  // LEFT
        case 8: p = 11; break; // UP+LEFT
        default: break;
    }

    float x     = (hd.lx - 0x8000) / 32768.0f;
    float y     = (hd.ly - 0x8000) / 32768.0f;
    float len   = sqrtf(x * x + y * y);
    float angle = 0;
    if (len > 0.4f) {
        angle = atan2f(y, x) / M_PI * 180.0f + 180.0f;
        p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
    }

    switch (p) {
        case 1: handctrl &= ~((1 << 1)); break;
        case 2: handctrl &= ~((1 << 4) | (1 << 1)); break;
        case 3: handctrl &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case 4: handctrl &= ~((1 << 1) | (1 << 0)); break;
        case 5: handctrl &= ~((1 << 0)); break;
        case 6: handctrl &= ~((1 << 4) | (1 << 0)); break;
        case 7: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        case 8: handctrl &= ~((1 << 3) | (1 << 0)); break;
        case 9: handctrl &= ~((1 << 3)); break;
        case 10: handctrl &= ~((1 << 4) | (1 << 3)); break;
        case 11: handctrl &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case 12: handctrl &= ~((1 << 3) | (1 << 2)); break;
        case 13: handctrl &= ~((1 << 2)); break;
        case 14: handctrl &= ~((1 << 4) | (1 << 2)); break;
        case 15: handctrl &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case 16: handctrl &= ~((1 << 2) | (1 << 1)); break;
        default: break;
    }

    static uint8_t old_handctrl = 0xFF;
    if (old_handctrl != handctrl) {
        FPGA::instance().aqpUpdateHandCtrl(handctrl, 0xFF, pdMS_TO_TICKS(100));
        old_handctrl = handctrl;
    }
}

void handleKeyboardData(const uint8_t buf[8]) {
    // Don't process during rollover
    if (buf[2] == 1)
        return;

    static uint8_t prev_status[8];

    auto &kb = AqKeyboard::instance();

    // Check for modifier key changes
    uint8_t mod_released = prev_status[0] & ~buf[0];
    if (mod_released & (1 << 0))
        kb.handleScancode(0xE0, false); // Left CTRL
    if (mod_released & (1 << 1))
        kb.handleScancode(0xE1, false); // Left SHIFT
    if (mod_released & (1 << 2))
        kb.handleScancode(0xE2, false); // Left ALT
    if (mod_released & (1 << 3))
        kb.handleScancode(0xE3, false); // Left GUI
    if (mod_released & (1 << 4))
        kb.handleScancode(0xE4, false); // Right CTRL
    if (mod_released & (1 << 5))
        kb.handleScancode(0xE5, false); // Right SHIFT
    if (mod_released & (1 << 6))
        kb.handleScancode(0xE6, false); // Right ALT
    if (mod_released & (1 << 7))
        kb.handleScancode(0xE7, false); // Right GUI

    uint8_t mod_pressed = ~prev_status[0] & buf[0];
    if (mod_pressed & (1 << 0))
        kb.handleScancode(0xE0, true); // Left CTRL
    if (mod_pressed & (1 << 1))
        kb.handleScancode(0xE1, true); // Left SHIFT
    if (mod_pressed & (1 << 2))
        kb.handleScancode(0xE2, true); // Left ALT
    if (mod_pressed & (1 << 3))
        kb.handleScancode(0xE3, true); // Left GUI
    if (mod_pressed & (1 << 4))
        kb.handleScancode(0xE4, true); // Right CTRL
    if (mod_pressed & (1 << 5))
        kb.handleScancode(0xE5, true); // Right SHIFT
    if (mod_pressed & (1 << 6))
        kb.handleScancode(0xE6, true); // Right ALT
    if (mod_pressed & (1 << 7))
        kb.handleScancode(0xE7, true); // Right GUI

    uint8_t prev, cur;

    // Check for key releases
    for (int j = 2; j < 8; j++) {
        if ((prev = prev_status[j]) == 0)
            break;

        // Check if this key is in current report
        bool key_released = true;
        for (int i = 2; i < 8; i++) {
            if ((cur = buf[i]) == 0)
                break;

            if (prev == cur) {
                key_released = false;
                break;
            }
        }

        if (key_released)
            kb.handleScancode(prev, false);
    }

    // Check for key presses
    for (int j = 2; j < 8; j++) {
        if ((cur = buf[j]) == 0)
            break;

        // Check if this key is in previous report
        bool key_pressed = true;
        for (int i = 2; i < 8; i++) {
            if ((prev = prev_status[i]) == 0)
                break;

            if (prev == cur) {
                key_pressed = false;
                break;
            }
        }

        if (key_pressed)
            kb.handleScancode(cur, true);
    }

    for (int i = 0; i < 8; i++) {
        prev_status[i] = buf[i];
    }

    kb.updateMatrix();
}
