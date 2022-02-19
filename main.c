#include "common.h"
#include <SDL.h>
#undef main

#include "emustate.h"

#include "video.h"
#include "keyboard.h"
#include "audio.h"
#include "ch376.h"

static uint8_t mem_read(size_t param, uint16_t addr) {
    (void)param;
    if (emustate.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    uint8_t result = 0xFF;
    if (addr < 0x3000) {
        result = emustate.rom[addr];
    } else if (addr >= 0x3000 && addr < 0x4000) {
        result = emustate.ram[addr - 0x3000];
    } else if (emustate.ramexp_enabled && addr >= 0x4000 && addr < 0xC000) {
        result = emustate.ramexp[addr - 0x4000];
    } else if (addr >= 0xC000) {
        result = emustate.gamerom[addr - 0xC000] ^ emustate.scramble_value;
    } else {
        printf("mem_read(0x%04x) -> 0x%02x\n", addr, result);
    }
    return result;
}

static void mem_write(size_t param, uint16_t addr, uint8_t data) {
    (void)param;
    if (emustate.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    if (addr >= 0x3000 && addr < 0x4000) {
        emustate.ram[addr - 0x3000] = data;
    } else if (emustate.ramexp_enabled && addr >= 0x4000 && addr < 0xC000) {
        emustate.ramexp[addr - 0x4000] = data;
    } else {
        printf("mem_write(0x%04x, 0x%02x)\n", addr, data);
    }
}

static uint8_t io_read(size_t param, ushort addr) {
    (void)param;
    uint8_t result = 0xFF;

    // Bruce Abbott's Micro-Expander CH376
    if ((addr & 0xC0) == 0x40) {
        if ((addr & 1) == 0)
            return ch376_read_data();
        else
            return ch376_read_status();
    }

    switch (addr & 0xFF) {
        case 0x40:
            break;
        case 0x41:
            break;

        case 0xF6:
        case 0xF7:
            if (emustate.expander_enabled) {
                // AY-3-8910 register read
                switch (emustate.ay_addr) {
                    case 14: result = emustate.handctrl1; break;
                    case 15: result = emustate.handctrl2; break;
                    default:
                        if (emustate.ay_addr < 14)
                            result = ay8910_read_reg(&emustate.ay_state, emustate.ay_addr);

                        // printf("AY-3-8910 R%u => 0x%02X\n", emustate.ay_addr, result);
                        break;
                }
            }
            break;

        case 0xFC: printf("Cassette port input (%04x) -> %02x\n", addr, result); break;
        case 0xFD: result = (emustate.linenr >= 224) ? 0 : 1; break;
        case 0xFE: printf("Clear to send status (%04x) -> %02x\n", addr, result); break;
        case 0xFF: {
            // Keyboard matrix. Selected rows are passed in the upper 8 address lines.
            uint8_t rows = addr >> 8;

            // Wire-AND all selected rows.
            result = 0xFF;
            for (int i = 0; i < 8; i++) {
                if ((rows & (1 << i)) == 0) {
                    result &= emustate.keyb_matrix[i];
                }
            }
            break;
        }
        default: printf("io_read(0x%02x) -> 0x%02x\n", addr & 0xFF, result); break;
    }
    return result;
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

    switch (addr & 0xFF) {
        case 0xF6:
            if (emustate.expander_enabled) {
                // AY-3-8910 register write
                if (emustate.ay_addr < 14)
                    ay8910_write_reg(&emustate.ay_state, emustate.ay_addr, data);

                // printf("AY-3-8910 R%u=0x%02X\n", emustate.ay_addr, data);
            }
            break;
        case 0xF7:
            if (emustate.expander_enabled) {
                // AY-3-8910 address latch
                emustate.ay_addr = data;
            }
            break;

        case 0xFC: emustate.audio_out = (data & 1) ? AUDIO_LEVEL : -AUDIO_LEVEL; break;
        case 0xFD: emustate.cpm_remap = (data & 1) != 0; break;
        case 0xFE: printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); break;
        case 0xFF:
            printf("Scramble value: 0x%02x\n", data);
            emustate.scramble_value = data;
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

    emustate.scramble_value = 0;
    emustate.cpm_remap      = false;

    ay8910_reset(&emustate.ay_state);
}

// 3579545 Hz -> 59659 cycles / frame
// 7159090 Hz -> 119318 cycles / frame

// 455x262=119210 -> 60.05 Hz
// 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
// 366 active pixels

#define HCYCLES_PER_LINE (455)
#define HCYCLES_PER_SAMPLE (149)

static void render_screen(SDL_Renderer *renderer) {
    draw_screen();

    static SDL_Texture *texture = NULL;
    if (texture == NULL) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, VIDEO_WIDTH, VIDEO_HEIGHT);
    }

    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);

    const uint8_t *fb = video_get_fb();

    for (int j = 0; j < VIDEO_HEIGHT; j++) {
        for (int i = 0; i < VIDEO_WIDTH; i++) {

            // Convert from RGB 2:2:2 to RGB 8:8:8
            uint8_t col222 = fb[j * VIDEO_WIDTH + i];

            unsigned r = (col222 >> 0) & 3;
            unsigned g = (col222 >> 2) & 3;
            unsigned b = (col222 >> 4) & 3;

            ((uint32_t *)((uintptr_t)pixels + j * pitch))[i] =
                (r << 22) | (r << 20) | (r << 18) | (r << 16) |
                (g << 14) | (g << 12) | (g << 10) | (g << 8) |
                (b << 6) | (b << 4) | (b << 2) | (b << 0);
        }
    }

    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);

    // Determine target size
    {
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // Retain aspect ratio
        int w1 = w / VIDEO_WIDTH * VIDEO_WIDTH;
        int h1 = w1 * VIDEO_HEIGHT / VIDEO_WIDTH;
        int h2 = h / VIDEO_HEIGHT * VIDEO_HEIGHT;
        int w2 = h2 * VIDEO_WIDTH / VIDEO_HEIGHT;

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

            emustate.line_hcycles += delta;
            emustate.sample_hcycles += delta;

            if (emustate.line_hcycles >= HCYCLES_PER_LINE) {
                emustate.line_hcycles -= HCYCLES_PER_LINE;

                emustate.linenr++;
                if (emustate.linenr == 262) {
                    render_screen(renderer);
                    emustate.linenr = 0;
                }
            }
        } while (emustate.sample_hcycles < HCYCLES_PER_SAMPLE);

        emustate.sample_hcycles -= HCYCLES_PER_SAMPLE;

        // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
        float samples = 0;
        for (int i = 0; i < 5; i++) {
            samples += ay8910_render(&emustate.ay_state);
        }
        samples /= 5.0f;

        abuf[aidx] = emustate.audio_out + (uint16_t)(samples * AUDIO_LEVEL);
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
            case 'u': ch376_init(optarg); break;
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

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        exit(1);
    }

    reset();
    audio_init();

    // Load system ROM
    {
        FILE *f = fopen(rom_path, "rb");
        if (f == NULL) {
            perror(rom_path);
            exit(1);
        }
        if (fread(emustate.rom, 1, sizeof(emustate.rom), f) < 8192) {
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
    ay8910_init(&emustate.ay_state);
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
