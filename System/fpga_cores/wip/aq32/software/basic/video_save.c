#include "video_save.h"
#include "common.h"

static struct regs_tram saved_tram;
static uint8_t          saved_chram[2048];
static uint16_t         saved_palette[64];
static uint32_t         saved_reg_gfx_ctrl;

void save_video(void) {
    saved_tram = *TRAM;
    for (int i = 0; i < 2048; i++)
        saved_chram[i] = CHRAM[i];
    for (int i = 0; i < 64; i++)
        saved_palette[i] = PALETTE[i];
    saved_reg_gfx_ctrl = GFX->CTRL;
}

void restore_video_text(void) {
    *TRAM = saved_tram;
}

void restore_video(void) {
    restore_video_text();
    for (int i = 0; i < 2048; i++)
        CHRAM[i] = saved_chram[i];
    for (int i = 0; i < 64; i++)
        PALETTE[i] = saved_palette[i];
    GFX->CTRL = saved_reg_gfx_ctrl;
}
