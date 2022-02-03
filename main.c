#include "common.h"
#include <SDL.h>
#include "audio.h"
#include "z80.h"

#define AUDIO_LEVEL (4095)

static Z80Context     z80context;
static uint8_t        rom[0x2000];
static uint8_t        gamerom[0x4000];
static uint8_t        ram[0x1000];
static uint8_t        extram[0x8000];
static bool           is_vsync       = false;
static uint8_t        keyb_matrix[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static uint8_t        scramble_value = 0;
static bool           cpm_remap      = false;
static const uint32_t palette[16]    = {
    0x101010, 0xf71010, 0x10f710, 0xf7ef10,
    0x2121de, 0xf710f7, 0x31c6c6, 0xf7f7f7,
    0xc6c6c6, 0x29adad, 0xc621c6, 0x42108c,
    0xf7f773, 0x10F710, 0x21ce42, 0x313131};
extern unsigned char charrom_bin[2048];
static int16_t       audio_out        = AUDIO_LEVEL;
static bool          expander_enabled = true;

static uint8_t ay_addr = 0;
static uint8_t ay_regs[14];
static uint8_t handctrl1 = 0xFF;
static uint8_t handctrl2 = 0xFF;

static uint8_t mem_read(size_t param, uint16_t addr) {
    if (cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    uint8_t result = 0xFF;
    if (addr < 0x2000) {
        result = rom[addr];
    } else if (addr >= 0x3000 && addr < 0x4000) {
        result = ram[addr - 0x3000];
    } else if (addr >= 0x4000 && addr < 0xC000) {
        result = extram[addr - 0x4000];
    } else if (addr >= 0xC000) {
        result = gamerom[addr - 0xC000] ^ scramble_value;
    } else {
        printf("mem_read(0x%04x) -> 0x%02x\n", addr, result);
    }
    return result;
}

static void mem_write(size_t param, uint16_t addr, uint8_t data) {
    if (cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    if (addr >= 0x3000 && addr < 0x4000) {
        ram[addr - 0x3000] = data;
    } else if (addr >= 0x4000 && addr < 0xC000) {
        extram[addr - 0x4000] = data;
    } else {
        printf("mem_write(0x%04x, 0x%02x)\n", addr, data);
    }
}

static uint8_t io_read(size_t param, ushort addr) {
    uint8_t result = 0xFF;

    switch (addr & 0xFF) {
        case 0xF6:
        case 0xF7:
            if (expander_enabled) {
                // AY-3-8910 register read
                switch (ay_addr) {
                    case 14: result = handctrl1; break;
                    case 15: result = handctrl2; break;
                    default:
                        if (ay_addr < 14)
                            result = ay_regs[ay_addr];

                        printf("AY-3-8910 R%u => 0x%02X\n", ay_addr, result);
                        break;
                }
            }
            break;

        case 0xFC: printf("Cassette port input (%04x) -> %02x\n", addr, result); break;
        case 0xFD: result = is_vsync ? 0 : 1; break;
        case 0xFE: printf("Clear to send status (%04x) -> %02x\n", addr, result); break;
        case 0xFF: {
            uint8_t rows = addr >> 8;

            result = 0xFF;

            for (int i = 0; i < 8; i++) {
                if ((rows & (1 << i)) == 0) {
                    result &= keyb_matrix[i];
                }
            }
            // printf("Keyboard port (%04x) -> %02x\n", addr, result);
            break;
        }
        default: printf("io_read(0x%02x) -> 0x%02x\n", addr & 0xFF, result); break;
    }

    return result;
}

static void io_write(size_t param, uint16_t addr, uint8_t data) {
    switch (addr & 0xFF) {
        case 0xF6:
            if (expander_enabled) {
                // AY-3-8910 register write
                if (ay_addr < 14)
                    ay_regs[ay_addr] = data;

                printf("AY-3-8910 R%u=0x%02X\n", ay_addr, data);
            }
            break;
        case 0xF7:
            if (expander_enabled) {
                // AY-3-8910 address latch
                ay_addr = data;
            }
            break;

        case 0xFC: audio_out = (data & 1) ? AUDIO_LEVEL : -AUDIO_LEVEL; break;
        case 0xFD: cpm_remap = (data & 1) != 0; break;
        case 0xFE: printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); break;
        case 0xFF:
            scramble_value = data;
            printf("Scramble: %02X\n", data);
            break;
        default: printf("io_write(0x%02x, 0x%02x)\n", addr & 0xFF, data); break;
    }
}

static void reset(void) {
    Z80RESET(&z80context);
    z80context.ioRead   = io_read;
    z80context.ioWrite  = io_write;
    z80context.memRead  = mem_read;
    z80context.memWrite = mem_write;

    scramble_value = 0;
    cpm_remap      = false;
}

static void keyboard_scancode(unsigned scancode, bool keydown) {
    // printf("%c: %d\n", keydown ? 'D' : 'U', scancode);

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

    static int handctrl_pressed = 0;

    int key = -1;
    switch (scancode) {
        case SDL_SCANCODE_UP: handctrl_pressed = (keydown) ? (handctrl_pressed | UP) : (handctrl_pressed & ~UP); break;
        case SDL_SCANCODE_DOWN: handctrl_pressed = (keydown) ? (handctrl_pressed | DOWN) : (handctrl_pressed & ~DOWN); break;
        case SDL_SCANCODE_LEFT: handctrl_pressed = (keydown) ? (handctrl_pressed | LEFT) : (handctrl_pressed & ~LEFT); break;
        case SDL_SCANCODE_RIGHT: handctrl_pressed = (keydown) ? (handctrl_pressed | RIGHT) : (handctrl_pressed & ~RIGHT); break;
        case SDL_SCANCODE_F1: handctrl_pressed = (keydown) ? (handctrl_pressed | K1) : (handctrl_pressed & ~K1); break;
        case SDL_SCANCODE_F2: handctrl_pressed = (keydown) ? (handctrl_pressed | K2) : (handctrl_pressed & ~K2); break;
        case SDL_SCANCODE_F3: handctrl_pressed = (keydown) ? (handctrl_pressed | K3) : (handctrl_pressed & ~K3); break;
        case SDL_SCANCODE_F4: handctrl_pressed = (keydown) ? (handctrl_pressed | K4) : (handctrl_pressed & ~K4); break;
        case SDL_SCANCODE_F5: handctrl_pressed = (keydown) ? (handctrl_pressed | K5) : (handctrl_pressed & ~K5); break;
        case SDL_SCANCODE_F6: handctrl_pressed = (keydown) ? (handctrl_pressed | K6) : (handctrl_pressed & ~K6); break;

        case SDL_SCANCODE_ESCAPE:
            if (keydown)
                reset();
            break;

        case SDL_SCANCODE_EQUALS: key = 0; break;
        case SDL_SCANCODE_BACKSPACE: key = 1; break;
        case SDL_SCANCODE_APOSTROPHE: key = 2; break;
        case SDL_SCANCODE_RETURN: key = 3; break;
        case SDL_SCANCODE_SEMICOLON: key = 4; break;
        case SDL_SCANCODE_PERIOD: key = 5; break;

        case SDL_SCANCODE_MINUS: key = 6; break;
        case SDL_SCANCODE_SLASH: key = 7; break;
        case SDL_SCANCODE_0: key = 8; break;
        case SDL_SCANCODE_P: key = 9; break;
        case SDL_SCANCODE_L: key = 10; break;
        case SDL_SCANCODE_COMMA: key = 11; break;

        case SDL_SCANCODE_9: key = 12; break;
        case SDL_SCANCODE_O: key = 13; break;
        case SDL_SCANCODE_K: key = 14; break;
        case SDL_SCANCODE_M: key = 15; break;
        case SDL_SCANCODE_N: key = 16; break;
        case SDL_SCANCODE_J: key = 17; break;

        case SDL_SCANCODE_8: key = 18; break;
        case SDL_SCANCODE_I: key = 19; break;
        case SDL_SCANCODE_7: key = 20; break;
        case SDL_SCANCODE_U: key = 21; break;
        case SDL_SCANCODE_H: key = 22; break;
        case SDL_SCANCODE_B: key = 23; break;

        case SDL_SCANCODE_6: key = 24; break;
        case SDL_SCANCODE_Y: key = 25; break;
        case SDL_SCANCODE_G: key = 26; break;
        case SDL_SCANCODE_V: key = 27; break;
        case SDL_SCANCODE_C: key = 28; break;
        case SDL_SCANCODE_F: key = 29; break;

        case SDL_SCANCODE_5: key = 30; break;
        case SDL_SCANCODE_T: key = 31; break;
        case SDL_SCANCODE_4: key = 32; break;
        case SDL_SCANCODE_R: key = 33; break;
        case SDL_SCANCODE_D: key = 34; break;
        case SDL_SCANCODE_X: key = 35; break;

        case SDL_SCANCODE_3: key = 36; break;
        case SDL_SCANCODE_E: key = 37; break;
        case SDL_SCANCODE_S: key = 38; break;
        case SDL_SCANCODE_Z: key = 39; break;
        case SDL_SCANCODE_SPACE: key = 40; break;
        case SDL_SCANCODE_A: key = 41; break;

        case SDL_SCANCODE_2: key = 42; break;
        case SDL_SCANCODE_W: key = 43; break;
        case SDL_SCANCODE_1: key = 44; break;
        case SDL_SCANCODE_Q: key = 45; break;
        case SDL_SCANCODE_LSHIFT: key = 46; break;
        case SDL_SCANCODE_LCTRL: key = 47; break;
    }

    handctrl1 = 0xFF;
    switch (handctrl_pressed & 0xF) {
        case LEFT: handctrl1 &= ~(1 << 3); break;
        case UP | LEFT: handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case UP: handctrl1 &= ~(1 << 2); break;
        case UP | RIGHT: handctrl1 &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case RIGHT: handctrl1 &= ~(1 << 1); break;
        case DOWN | RIGHT: handctrl1 &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case DOWN: handctrl1 &= ~(1 << 0); break;
        case DOWN | LEFT: handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        default: break;
    }
    if (handctrl_pressed & K1)
        handctrl1 &= ~(1 << 6);
    if (handctrl_pressed & K2)
        handctrl1 &= ~((1 << 7) | (1 << 2));
    if (handctrl_pressed & K3)
        handctrl1 &= ~((1 << 7) | (1 << 5));
    if (handctrl_pressed & K4)
        handctrl1 &= ~(1 << 5);
    if (handctrl_pressed & K5)
        handctrl1 &= ~((1 << 7) | (1 << 1));
    if (handctrl_pressed & K6)
        handctrl1 &= ~((1 << 7) | (1 << 0));

    if (key < 0) {
        return;
    }

    if (keydown) {
        keyb_matrix[key / 6] &= ~(1 << (key % 6));
    } else {
        keyb_matrix[key / 6] |= (1 << (key % 6));
    }
}

static inline void draw_char(void *pixels, int pitch, uint8_t ch, uint8_t color, int row, int column) {
    uint32_t       fgcol = palette[color >> 4];
    uint32_t       bgcol = palette[color & 0xF];
    const uint8_t *ps    = &charrom_bin[ch * 8];

    for (int j = 0; j < 8; j++) {
        uint32_t *pd = &((uint32_t *)((uintptr_t)pixels + (row * 8 + j) * pitch))[column * 8];
        for (int i = 0; i < 8; i++) {
            pd[i] = (ps[j] & (1 << (7 - i))) ? fgcol : bgcol;
        }
    }
}

static void draw_screen(void *pixels, int pitch) {
    uint8_t border_ch    = ram[0];
    uint8_t border_color = ram[0x400];

    for (int row = 0; row < 28; row++) {
        for (int column = 0; column < 44; column++) {
            if (row >= 2 && row < 26 && column >= 2 && column < 42)
                continue;

            draw_char(pixels, pitch, border_ch, border_color, row, column);
        }
    }

    for (int row = 0; row < 24; row++) {
        for (int column = 0; column < 40; column++) {
            uint8_t ch    = ram[(row + 1) * 40 + column];
            uint8_t color = ram[0x400 + (row + 1) * 40 + column];
            draw_char(pixels, pitch, ch, color, row + 2, column + 2);
        }
    }
}

static void render_screen(SDL_Renderer *renderer) {
    static SDL_Texture *texture = NULL;
    if (texture == NULL) {
        texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 352, 224);
    }

    void *pixels;
    int   pitch;
    SDL_LockTexture(texture, NULL, &pixels, &pitch);
    draw_screen(pixels, pitch);
    SDL_UnlockTexture(texture);
    SDL_RenderClear(renderer);

    // Determine target size
    {
        int w, h;
        SDL_GetRendererOutputSize(renderer, &w, &h);

        // Retain aspect ratio
        int w1 = w / 352 * 352;
        int h1 = w1 * 224 / 352;
        int h2 = h / 224 * 224;
        int w2 = h2 * 352 / 224;

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

int main(int argc, char *argv[]) {
    char *base_path = SDL_GetBasePath();

    char rom_path[1024];
    snprintf(rom_path, sizeof(rom_path), "%s%s", base_path, "aquarius.rom");

    char cartrom_path[1024];
    cartrom_path[0] = 0;

    int  opt;
    bool params_ok = true;
    while ((opt = getopt(argc, argv, "r:c:x")) != -1) {
        switch (opt) {
            case 'r': snprintf(rom_path, sizeof(rom_path), "%s", optarg); break;
            case 'c': snprintf(cartrom_path, sizeof(cartrom_path), "%s", optarg); break;
            case 'x': expander_enabled = false; break;
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
        fprintf(stderr, "-x          Disable mini expander emulation.\n");
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
        fread(rom, sizeof(rom), 1, f);
        fclose(f);
    }

    // Load cartridge ROM
    if (cartrom_path[0] != 0) {
        FILE *f = fopen(cartrom_path, "rb");
        if (f == NULL) {
            perror(cartrom_path);
            exit(1);
        }
        fread(gamerom, sizeof(gamerom), 1, f);
        fclose(f);
    }

    unsigned long wnd_width = 352 * 2, wnd_height = 224 * 2;
    SDL_Window *  window = SDL_CreateWindow("Aquarius emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, wnd_width, wnd_height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);
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

    int frame_cycles      = 0;
    int sample_halfcycles = 0;

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

            case SDL_USEREVENT: {
                // Render screen
                int16_t *abuf = audio_get_buffer();
                int      aidx = 0;

                // 3579545 Hz -> 59659 cycles / frame
                // 7159090 Hz -> 119318 cycles / frame

                // 455x262=119210 -> 60.05 Hz
                // 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
                // 366 active pixels

                const int cycles_per_frame      = 59659;
                const int halfcycles_per_sample = 149;

                while (frame_cycles < 49596) {
                    unsigned tstates = z80context.tstates;
                    Z80Execute(&z80context);
                    int delta = z80context.tstates - tstates;
                    frame_cycles += delta;

                    sample_halfcycles += delta * 2;
                    if (sample_halfcycles >= halfcycles_per_sample) {
                        sample_halfcycles -= halfcycles_per_sample;
                        assert(aidx < 800);

                        if (abuf != NULL)
                            abuf[aidx] = audio_out;
                        aidx++;
                    }
                }

                render_screen(renderer);
                is_vsync = true;
                while (frame_cycles < cycles_per_frame) {
                    unsigned tstates = z80context.tstates;
                    Z80Execute(&z80context);
                    int delta = z80context.tstates - tstates;
                    frame_cycles += delta;

                    sample_halfcycles += delta * 2;
                    if (sample_halfcycles >= halfcycles_per_sample) {
                        sample_halfcycles -= halfcycles_per_sample;

                        if (abuf != NULL)
                            abuf[aidx] = audio_out;
                        aidx++;
                        if (aidx >= 800)
                            break;
                    }
                }
                is_vsync = false;

                frame_cycles = 0;

                if (abuf != NULL)
                    audio_put_buffer(abuf);

                break;
            }
        }
    }
    return 0;
}
