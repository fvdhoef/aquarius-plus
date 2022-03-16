#include "common.h"
#include <SDL.h>
#undef main

#include "emustate.h"

#include "video.h"
#include "keyboard.h"
#include "audio.h"
#include "ch376.h"
#include "esp32.h"

static uint8_t mem_read(size_t param, uint16_t addr) {
    (void)param;

    // Handle CPM remap bit
    if (emustate.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankreg     = emustate.bankregs[addr >> 14];
    unsigned page        = bankreg & 0x3F;
    bool     overlay_ram = (bankreg & (1 << 6)) != 0;

    addr &= 0x3FFF;

    if (overlay_ram && addr >= 0x3000) {
        if (addr < 0x3400) {
            return emustate.screenram[addr & 0x3FF];
        } else if (addr < 0x3800) {
            return emustate.colorram[addr & 0x3FF];
        } else {
            return emustate.basicram[addr & 0x7FF];
        }
    }

    if (page < 16) {
        return emustate.flashrom[page * 0x4000 + addr];
    } else if (page == 19) {
        return emustate.gamerom[addr] ^ emustate.extbus_scramble;
    } else if (page == 20) {
        return emustate.videoram[addr];
    } else if (page == 21) {
        if (addr < 0x800) {
            return emustate.charram[addr];
        }
    } else if (page >= 32 && page < 64) {
        return emustate.mainram[(page - 32) * 0x4000 + addr];
    }
    return 0xFF;
}

static void mem_write(size_t param, uint16_t addr, uint8_t data) {
    (void)param;

    // Handle CPM remap bit
    if (emustate.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    // Get and decode banking register
    uint8_t  bankreg     = emustate.bankregs[addr >> 14];
    unsigned page        = bankreg & 0x3F;
    bool     overlay_ram = (bankreg & (1 << 6)) != 0;
    bool     readonly    = (bankreg & (1 << 7)) != 0;
    addr &= 0x3FFF;

    if (overlay_ram && addr >= 0x3000) {
        if (addr < 0x3400) {
            emustate.screenram[addr & 0x3FF] = data;
        } else if (addr < 0x3800) {
            emustate.colorram[addr & 0x3FF] = data;
        } else {
            emustate.basicram[addr & 0x7FF] = data;
        }
        return;
    }

    if (readonly) {
        return;
    }

    if (page < 16) {
        // Flash memory is read-only for now
        // TODO: Implement flash emulation
        return;
    } else if (page == 19) {
        // Game ROM is readonly
        return;
    } else if (page == 20) {
        emustate.videoram[addr] = data;
    } else if (page == 21) {
        if (addr < 0x800) {
            emustate.charram[addr] = data;
        }
    } else if (page >= 32 && page < 64) {
        emustate.mainram[(page - 32) * 0x4000 + addr] = data;
    }
}

static uint8_t io_read(size_t param, ushort addr) {
    (void)param;

    // Bruce Abbott's Micro-Expander CH376
    if ((addr & 0xC0) == 0x40) {
        if ((addr & 1) == 0)
            return ch376_read_data();
        else
            return ch376_read_status();
    }

    if (!emustate.sysctrl_disable_ext) {
        switch (addr & 0xFF) {
            case 0xE0: return emustate.video_ctrl;
            case 0xE1: return emustate.video_scrx & 0xFF;
            case 0xE2: return emustate.video_scrx >> 8;
            case 0xE3: return emustate.video_scry;
            case 0xE4: return emustate.video_sprx[(addr >> 8) & 0x3F] & 0xFF;
            case 0xE5: return emustate.video_sprx[(addr >> 8) & 0x3F] >> 8;
            case 0xE6: return emustate.video_spry[(addr >> 8) & 0x3F];
            case 0xE7: return emustate.video_spridx[(addr >> 8) & 0x3F] & 0xFF;
            case 0xE8: return (emustate.video_sprattr[(addr >> 8) & 0x3F] & 0xFE) | ((emustate.video_spridx[(addr >> 8) & 0x3F] >> 8) & 1);
            case 0xE9:
                if ((addr & (1 << 8)) == 0)
                    return emustate.video_palette_text[(addr >> 9) & 0xF] & 0xFF;
                else
                    return emustate.video_palette_text[(addr >> 9) & 0xF] >> 8;

            case 0xEA:
                if ((addr & (1 << 8)) == 0)
                    return emustate.video_palette_tile[(addr >> 9) & 0xF] & 0xFF;
                else
                    return emustate.video_palette_tile[(addr >> 9) & 0xF] >> 8;

            case 0xEB:
                if ((addr & (1 << 8)) == 0)
                    return emustate.video_palette_sprite[(addr >> 9) & 0xF] & 0xFF;
                else
                    return emustate.video_palette_sprite[(addr >> 9) & 0xF] >> 8;

            case 0xEC: return emustate.video_line < 255 ? emustate.video_line : 255;
            case 0xED: return emustate.video_irqline;
            case 0xEE: return emustate.irqmask;
            case 0xEF: return emustate.irqstatus;
            case 0xF0: return emustate.bankregs[0];
            case 0xF1: return emustate.bankregs[1];
            case 0xF2: return emustate.bankregs[2];
            case 0xF3: return emustate.bankregs[3];
            case 0xF4: return esp32_read_ctrl();
            case 0xF5: return esp32_read_data();
        }
    }

    switch (addr & 0xFF) {
        case 0xF6:
        case 0xF7:
            if (emustate.sysctrl_ay_disable)
                return 0xFF;
            else {
                switch (emustate.ay_addr) {
                    case 14: return emustate.handctrl1; break;
                    case 15: return emustate.handctrl2; break;
                    default: return ay8910_read_reg(&emustate.ay_state, emustate.ay_addr);
                }
            }
            break;

        case 0xF8:
        case 0xF9:
            if (emustate.sysctrl_ay_disable || emustate.sysctrl_disable_ext)
                return 0xFF;
            else
                return ay8910_read_reg(&emustate.ay2_state, emustate.ay2_addr);

        case 0xFB: return (
            (emustate.sysctrl_disable_ext ? (1 << 0) : 0) |
            (emustate.sysctrl_ay_disable ? (1 << 1) : 0));

        case 0xFC: printf("Cassette port input (%04x)\n", addr); return 0xFF;
        case 0xFD: return (emustate.video_line >= 242) ? 0 : 1;
        case 0xFE: printf("Clear to send status (%04x)\n", addr); return 0xFF;
        case 0xFF: {
            // Keyboard matrix. Selected rows are passed in the upper 8 address lines.
            uint8_t rows = addr >> 8;

            // Wire-AND all selected rows.
            uint8_t result = 0xFF;
            for (int i = 0; i < 8; i++) {
                if ((rows & (1 << i)) == 0) {
                    result &= emustate.keyb_matrix[i];
                }
            }
            return result;
        }
        default: break;
    }

    printf("io_read(0x%02x)\n", addr & 0xFF);
    return 0xFF;
}

static void io_write(size_t param, uint16_t addr, uint8_t data) {
    (void)param;

    // Bruce Abbott's Micro-Expander CH376
    if ((addr & 0xC0) == 0x40) {
        if ((addr & 1) == 0)
            ch376_write_data(data);
        else
            ch376_write_cmd(data);
        return;
    }

    if (!emustate.sysctrl_disable_ext) {
        switch (addr & 0xFF) {
            case 0xE0: emustate.video_ctrl = data; return;
            case 0xE1:
                // emustate.video_scrx = (emustate.video_scrx & ~0xFF) | data; return;
                emustate.tmp_latch = data;
                return;
            case 0xE2:
                // emustate.video_scrx = (emustate.video_scrx & 0xFF) | ((data & 1) << 8);
                emustate.video_scrx = emustate.tmp_latch | ((data & 1) << 8);
                return;
            case 0xE3: emustate.video_scry = data; return;
            case 0xE4:
                emustate.tmp_latch = data;
                return;
            case 0xE5:
                //  emustate.video_sprx[(addr >> 8) & 0x3F] = (emustate.video_sprx[(addr >> 8) & 0x3F] & ~0xFF) | data; return;
                emustate.video_sprx[(addr >> 8) & 0x3F] = ((data & 1) << 8) | emustate.tmp_latch;
                return;
            case 0xE6: emustate.video_spry[(addr >> 8) & 0x3F] = data; return;
            case 0xE7: emustate.video_spridx[(addr >> 8) & 0x3F] = (emustate.video_spridx[(addr >> 8) & 0x3F] & ~0xFF) | data; return;
            case 0xE8:
                emustate.video_sprattr[(addr >> 8) & 0x3F] = data & 0xFE;
                emustate.video_spridx[(addr >> 8) & 0x3F]  = (emustate.video_spridx[(addr >> 8) & 0x3F] & 0xFF) | ((data & 1) << 8);
                return;
            case 0xE9:
                if ((addr & (1 << 8)) == 0)
                    emustate.tmp_latch = data;
                else
                    emustate.video_palette_text[(addr >> 9) & 0x0F] = ((data & 0xF) << 8) | emustate.tmp_latch;
                return;
            case 0xEA:
                if ((addr & (1 << 8)) == 0)
                    emustate.tmp_latch = data;
                else
                    emustate.video_palette_tile[(addr >> 9) & 0x0F] = ((data & 0xF) << 8) | emustate.tmp_latch;
                return;
            case 0xEB:
                if ((addr & (1 << 8)) == 0)
                    emustate.tmp_latch = data;
                else
                    emustate.video_palette_sprite[(addr >> 9) & 0x0F] = ((data & 0xF) << 8) | emustate.tmp_latch;
                return;
            case 0xEC: return;
            case 0xED: emustate.video_irqline = data; return;
            case 0xEE: emustate.irqmask = data & 3; return;
            case 0xEF: emustate.irqstatus &= ~data; return;
            case 0xF0: emustate.bankregs[0] = data; return;
            case 0xF1: emustate.bankregs[1] = data; return;
            case 0xF2: emustate.bankregs[2] = data; return;
            case 0xF3: emustate.bankregs[3] = data; return;
            case 0xF4: esp32_write_ctrl(data); return;
            case 0xF5: esp32_write_data(data); return;
        }
    }

    switch (addr & 0xFF) {
        case 0xF6:
            if (!emustate.sysctrl_ay_disable && emustate.ay_addr < 14)
                ay8910_write_reg(&emustate.ay_state, emustate.ay_addr, data);
            return;

        case 0xF7:
            if (!emustate.sysctrl_ay_disable)
                emustate.ay_addr = data;
            return;

        case 0xF8:
            if (!(emustate.sysctrl_ay_disable || emustate.sysctrl_disable_ext) && emustate.ay2_addr < 14)
                ay8910_write_reg(&emustate.ay2_state, emustate.ay2_addr, data);
            return;

        case 0xF9:
            if (!(emustate.sysctrl_ay_disable || emustate.sysctrl_disable_ext))
                emustate.ay2_addr = data;
            return;

        case 0xFB:
            emustate.sysctrl_disable_ext = (data & (1 << 0)) != 0;
            emustate.sysctrl_ay_disable  = (data & (1 << 1)) != 0;
            return;

        case 0xFC: emustate.sound_output = (data & 1) != 0; break;
        case 0xFD: emustate.cpm_remap = (data & 1) != 0; break;
        case 0xFE: printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); break;
        case 0xFF:
            printf("Scramble value: 0x%02x\n", data);
            emustate.extbus_scramble = data;
            break;
        default: printf("io_write(0x%02x, 0x%02x)\n", addr & 0xFF, data); break;
    }
}

void reset(void) {
    Z80RESET(&emustate.z80context);
    emustate.z80context.ioRead   = io_read;
    emustate.z80context.ioWrite  = io_write;
    emustate.z80context.memRead  = mem_read;
    emustate.z80context.memWrite = mem_write;

    emustate.extbus_scramble = 0;
    emustate.cpm_remap       = false;

    ay8910_reset(&emustate.ay_state);
    ay8910_reset(&emustate.ay2_state);
}

// 3579545 Hz -> 59659 cycles / frame
// 7159090 Hz -> 119318 cycles / frame

// 455x262=119210 -> 60.05 Hz
// 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
// 366 active pixels

#define HCYCLES_PER_LINE (455)
#define HCYCLES_PER_SAMPLE (162)

static void render_screen(SDL_Renderer *renderer) {
    static SDL_Texture *texture = NULL;
    if (texture == NULL) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);
    }

    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    const uint16_t *fb = video_get_fb();

    for (int j = 0; j < VIDEO_HEIGHT; j++) {
        for (int i = 0; i < VIDEO_WIDTH; i++) {

            // Convert from RGB444 to RGB888
            uint16_t col444 = fb[j * VIDEO_WIDTH + i];

            unsigned r4 = (col444 >> 8) & 0xF;
            unsigned g4 = (col444 >> 4) & 0xF;
            unsigned b4 = (col444 >> 0) & 0xF;

            unsigned r8 = (r4 << 4) | r4;
            unsigned g8 = (g4 << 4) | g4;
            unsigned b8 = (b4 << 4) | b4;

            ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] = (r8 << 16) | (g8 << 8) | (b8);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);

    // Determine target size
    {
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // Retain aspect ratio
        int w1 = (w / VIDEO_WIDTH) * VIDEO_WIDTH;
        int h1 = (w1 * VIDEO_HEIGHT) / VIDEO_WIDTH;
        int h2 = (h / VIDEO_HEIGHT) * VIDEO_HEIGHT;
        int w2 = (h2 * VIDEO_WIDTH) / VIDEO_HEIGHT;

        int sw, sh;
        if (w1 == 0 || h1 == 0) {
            sw = w;
            sh = h;
        } else if (w1 <= w && h1 <= h) {
            sw = w1;
            sh = h1;
        } else {
            sw = w2;
            sh = h2;
        }

        SDL_Rect dst;
        dst.w = (int)sw;
        dst.h = (int)sh;
        dst.x = (w - dst.w) / 2;
        dst.y = (h - dst.h) / 2;
        SDL_RenderCopy(renderer, texture, NULL, &dst);
    }
    SDL_RenderPresent(renderer);
}

static void emulate(SDL_Renderer *renderer) {
    // Emulation is performed in sync with the audio. This function will run
    // for the time needed to fill 1 audio buffer, which is about 1/60 of a
    // second.

    // Get a buffer from audio subsystem.
    uint16_t *abuf = audio_get_buffer();
    if (abuf == NULL) {
        // No buffer available, don't emulate for now.
        return;
    }

    // Render each audio sample
    for (int aidx = 0; aidx < SAMPLES_PER_BUFFER; aidx++) {
        do {
            emustate.z80context.tstates = 0;
            Z80Execute(&emustate.z80context);
            int delta = emustate.z80context.tstates * 2;

            int old_line_hcycles = emustate.line_hcycles;

            emustate.line_hcycles += delta;
            emustate.sample_hcycles += delta;

            if (old_line_hcycles < 320 && emustate.line_hcycles >= 320) {
                emustate.irqstatus |= (1 << 1);
            }

            if (emustate.line_hcycles >= HCYCLES_PER_LINE) {
                emustate.line_hcycles -= HCYCLES_PER_LINE;

                video_draw_line();

                emustate.video_line++;
                if (emustate.video_line == 200) {
                    emustate.irqstatus |= (1 << 0);

                } else if (emustate.video_line == 262) {
                    render_screen(renderer);
                    emustate.video_line = 0;
                }
            }

            if ((emustate.irqstatus & emustate.irqmask) != 0) {
                Z80INT(&emustate.z80context, 0xFF);
            }

        } while (emustate.sample_hcycles < HCYCLES_PER_SAMPLE);

        emustate.sample_hcycles -= HCYCLES_PER_SAMPLE;

        uint16_t beep = emustate.sound_output ? AUDIO_LEVEL : 0;

        // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
        unsigned left  = 0;
        unsigned right = 0;

        for (int i = 0; i < 5; i++) {
            uint16_t abc[3];
            ay8910_render(&emustate.ay_state, abc);
            left += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            right += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];

            ay8910_render(&emustate.ay2_state, abc);
            left += 2 * abc[0] + 2 * abc[1] + 1 * abc[2];
            right += 1 * abc[0] + 2 * abc[1] + 2 * abc[2];
        }

        left  = left + beep;
        right = right + beep;

        abuf[aidx * 2 + 0] = left;
        abuf[aidx * 2 + 1] = right;
    }

    // Return buffer to audio subsystem.
    audio_put_buffer(abuf);
}

int main(int argc, char *argv[]) {
    char *base_path = SDL_GetBasePath();

    char rom_path[1024];
    snprintf(rom_path, sizeof(rom_path), "%s%s", base_path, "aquarius.rom");

    char cartrom_path[1024];
    cartrom_path[0] = 0;

    int  opt;
    bool params_ok = true;
    while ((opt = getopt(argc, argv, "r:c:XRu:")) != -1) {
        if (opt == '?' || opt == ':') {
            params_ok = false;
            break;
        }
        switch (opt) {
            case 'r': snprintf(rom_path, sizeof(rom_path), "%s", optarg); break;
            case 'c': snprintf(cartrom_path, sizeof(cartrom_path), "%s", optarg); break;
            case 'X': emustate.expander_enabled = false; break;
            case 'R': emustate.ramexp_enabled = false; break;
            case 'u':
                ch376_init(optarg);
                esp32_init(optarg);
                break;
            default: params_ok = false; break;
        }
    }
    if (optind != argc) {
        params_ok = false;
    }

    if (!params_ok) {
        fprintf(stderr, "Usage: %s <options>\n\n", argv[0]);
        fprintf(stderr, "Options:\n");
        fprintf(stderr, "-r <path>   Set system ROM image path (default: %saquarius.rom)\n", base_path);
        fprintf(stderr, "-c <path>   Set cartridge ROM path\n");
        fprintf(stderr, "-u <path>   CH376 USB base path\n");
        fprintf(stderr, "-X          Disable mini expander emulation.\n");
        fprintf(stderr, "-R          Disable RAM expansion.\n");
        fprintf(stderr, "\n");
        exit(1);
    }

    emustate_init();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    // Load system ROM
    {
        FILE *f = fopen(rom_path, "rb");
        if (f == NULL) {
            perror(rom_path);
            exit(1);
        }
        if (fread(emustate.flashrom, 1, sizeof(emustate.flashrom), f) < 8192) {
            fprintf(stderr, "Error during reading of system ROM image.\n");
            exit(1);
        }
        fclose(f);
    }

    // Load cartridge ROM
    if (cartrom_path[0] != 0) {
        FILE *f = fopen(cartrom_path, "rb");
        if (f == NULL) {
            perror(cartrom_path);
            exit(1);
        }

        fseek(f, 0, SEEK_END);
        int filesize = ftell(f);
        fseek(f, 0, SEEK_SET);

        if (filesize == 8192) {
            if (fread(emustate.gamerom + 8192, filesize, 1, f) <= 0) {
                fprintf(stderr, "Error during reading of cartridge ROM image.\n");
                exit(1);
            }
            // Mirror ROM to $C000
            memcpy(emustate.gamerom, emustate.gamerom + 8192, 8192);

        } else if (filesize == 16384) {
            if (fread(emustate.gamerom, filesize, 1, f) <= 0) {
                fprintf(stderr, "Error during reading of cartridge ROM image.\n");
                exit(1);
            }

        } else {
            fprintf(stderr, "Invalid cartridge ROM file: %u, should be either exactly 8 or 16KB.\n", filesize);
        }

        fclose(f);
    }

    unsigned long wnd_width = VIDEO_WIDTH * 2, wnd_height = VIDEO_HEIGHT * 2;
    SDL_Window   *window = SDL_CreateWindow("Aquarius emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wnd_width, wnd_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Event event;
    audio_init();
    reset();
    audio_start();

    while (SDL_WaitEvent(&event) != 0 && event.type != SDL_QUIT) {
        switch (event.type) {
            case SDL_KEYDOWN:
            case SDL_KEYUP: {
                if (!event.key.repeat && event.key.keysym.scancode <= 255) {
                    keyboard_scancode(event.key.keysym.scancode, event.type == SDL_KEYDOWN);
                }
                break;
            }
            case SDL_USEREVENT: emulate(renderer); break;
        }
    }
    return 0;
}
