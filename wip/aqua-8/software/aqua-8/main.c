#include "common.h"
#include "scr_console/console.h"
#include <sys/stat.h>
#include <errno.h>

#include "trap.h"
#include "esp.h"
#include "scr.h"

volatile bool frame60   = false;
volatile bool frame30   = false;
unsigned      frame_cnt = 0;

void vblank_handler(void) {
    frame_cnt++;

    if (frame_cnt & 1) {
        frame30 = true;
    }
    frame60 = true;
}

void trap_handler(struct trap_regs *regs) {
    if ((int)regs->cause >= 0) {
        while (1);
    }

    unsigned mip = csr_read_clear(mip, -1UL);
    if (mip & (1 << VBLANK_IRQn)) {
        vblank_handler();
    }
}

static void handle_mouse(void) {
    static uint8_t prev_buttons = 0;

    esp_cmd(ESPCMD_GETMOUSE);
    uint8_t result = esp_get_byte();
    if (result == 0) {
        uint16_t x = esp_get_byte();
        x |= esp_get_byte() << 8;
        uint8_t y       = esp_get_byte();
        uint8_t buttons = esp_get_byte();
        int8_t  wheel   = esp_get_byte();

        if (x > 240)
            x = 240;

        // char bla[32];
        // snprintf(bla, sizeof(bla), "%u %u %u %d", x, y, buttons, wheel);
        // draw_text(bla, 50, 70, 7, false);

        edit_state.mouse_spr = MOUSE_SPR_POINTER;

        uint8_t clicked_buttons = ~prev_buttons & buttons;
        prev_buttons            = buttons;

        edit_state.mouse_ev.x               = x;
        edit_state.mouse_ev.y               = y;
        edit_state.mouse_ev.buttons         = buttons;
        edit_state.mouse_ev.clicked_buttons = clicked_buttons;
        edit_state.mouse_ev.wheel           = wheel;
    }
}

static void draw_mouse_cursor(void) {
    int sx = edit_state.mouse_ev.x;
    int sy = edit_state.mouse_ev.y;
    switch (edit_state.mouse_spr) {
        case MOUSE_SPR_POINTER:
            sx -= 1;
            sy -= 1;
            break;
        case MOUSE_SPR_HAND:
            sx -= 3;
            sy -= 1;
            break;
        case MOUSE_SPR_IBEAM:
            sx -= 1;
            sy -= 4;
            break;
        case MOUSE_SPR_CROSSHAIR:
            sx -= 3;
            sy -= 3;
            break;
        default: break;
    }
    draw_sprite(edit_state.mouse_spr, sx, sy);
}

static void handle_keybuf(void) {
    static uint16_t lastKeys[16];
    while (1) {
        int keybuf = KEYBUF;
        if (keybuf < 0)
            break;

        for (int i = 0; i < 15; i++) {
            lastKeys[i] = lastKeys[i + 1];
        }
        lastKeys[15]         = keybuf;
        edit_state.modifiers = keybuf & KEY_MODIFIERS;
        scr_key(keybuf);

        // last = keybuf;
    }
}

int main(void) {
    esp_closeall();
    palette_init();
    remap_reset();

    __irq_enable();
    csr_write(mie, (1 << VBLANK_IRQn));

    for (unsigned mode = MODE_CODE; mode <= MODE_MUSIC; mode++) {
        screen_t *scr = scr_get(mode);
        if (scr->init)
            scr->init();
    }

    while (1) {
        if (!edit_state.editing) {
            console_perform();
        } else {
            unsigned page = 2;
            VIDEO->PAGE   = page;

            while (edit_state.editing) {
                // VIDEO->PALETTE[1] = 0x222;
                handle_keybuf();
                handle_mouse();

                edit_state.status_text[0] = 0;
                scr_get_current()->draw();
                scr_draw_status();
                draw_mouse_cursor();
                edit_state.mouse_ev.clicked_buttons = 0;
                edit_state.mouse_ev.wheel           = 0;

                palette_init();

                frame60 = false;
                while (!frame60) {
                }
                page ^= 3;
                VIDEO->PAGE = page;
            }
        }
    }

    return 0;
}
