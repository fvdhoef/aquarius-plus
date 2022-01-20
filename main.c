#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include <SDL.h>

#include "z80.h"

static Z80Context     z80context;
static uint8_t        rom[0x2000];
static uint8_t        gamerom[0x4000];
static uint8_t        ram[0x1000];
static uint8_t        extram[0x8000];
static volatile bool  frame_busy     = false;
static uint8_t        scramble_value = 0;
static volatile bool  is_vsync       = false;
static uint8_t        keyb_matrix[8] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static const uint32_t palette[16]    = {
    0x101010, 0xf71010, 0x10f710, 0xf7ef10,
    0x2121de, 0xf710f7, 0x31c6c6, 0xf7f7f7,
    0xc6c6c6, 0x29adad, 0xc621c6, 0x42108c,
    0xf7f773, 0x10F710, 0x21ce42, 0x313131};
extern unsigned char charset[256 * 8 * 8];

static uint8_t mem_read(size_t param, uint16_t addr) {
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
        case 0xF7: break;

        case 0xFC: printf("Cassette port input (%04x) -> %02x\n", addr, result); break;
        case 0xFD:
            result = is_vsync ? 1 : 0;
            //  printf("Vertical sync signal (%04x) -> %02x\n", addr, result);
            break;
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
        default: printf("io_read(0x%04x) -> 0x%02x\n", addr, result); break;
    }

    return result;
}

static void io_write(size_t param, uint16_t addr, uint8_t data) {
    switch (addr & 0xFF) {
        case 0xF6:
        case 0xF7:
            // AY-3-8910
            break;

        case 0xFC:
            // printf("Cassette and sound port output (%04x) = %02x\n", addr, data);
            break;
        case 0xFD: printf("CP/M mode memory mapper (%04x) = %02x\n", addr, data); break;
        case 0xFE: printf("1200 bps serial printer (%04x) = %02x\n", addr, data); break;
        case 0xFF:
            // printf("Software lock pattern (%04x) = %02x\n", addr, data);
            scramble_value = data;
            break;
        default: printf("io_write(0x%04x, 0x%02x)\n", addr, data); break;
    }
}

static Uint32 timer_callback(Uint32 interval, void *param) {
    (void)param;
    if (!frame_busy) {
        frame_busy = true;

        SDL_Event event;
        memset(&event, 0, sizeof(event));
        event.type      = SDL_USEREVENT;
        event.user.type = SDL_USEREVENT;
        event.user.code = 0;
        SDL_PushEvent(&event);
    }
    return interval;
}

static void keyboard_scancode(unsigned scancode, bool keydown) {
    // printf("%c: %d\n", keydown ? 'D' : 'U', scancode);

    int key = -1;
    switch (scancode) {
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

    if (key < 0) {
        return;
    }

    if (keydown) {
        keyb_matrix[key / 6] &= ~(1 << (key % 6));
    } else {
        keyb_matrix[key / 6] |= (1 << (key % 6));
    }
}

static inline void draw_char(uint32_t *pixels, int pitch, uint8_t ch, uint8_t color, int row, int column) {
    uint32_t fgcol = palette[color >> 4];
    uint32_t bgcol = palette[color & 0xF];

    const uint8_t *ps = &charset[ch * 64];

    for (int j = 0; j < 8; j++) {
        uint32_t *pd = &((uint32_t *)((uintptr_t)pixels + (row * 8 + j) * pitch))[column * 8];
        for (int i = 0; i < 8; i++) {
            pd[i] = ps[j * 8 + i] ? fgcol : bgcol;
        }
    }
}

int main(int argc, const char **argv) {
    Z80RESET(&z80context);
    z80context.ioRead   = io_read;
    z80context.ioWrite  = io_write;
    z80context.memRead  = mem_read;
    z80context.memWrite = mem_write;
    {
        FILE *f = fopen("aquarius.rom", "rb");
        if (f == NULL) {
            perror("aquarius.rom");
            exit(1);
        }
        fread(rom, sizeof(rom), 1, f);
        fclose(f);
    }

    if (1) {
        // FILE *f = fopen("roms/Astrosmash (1982)(Mattel).bin", "rb");
        // FILE *f = fopen("roms/Advanced Dungeons & Dragons (1982)(Mattel).bin", "rb");
        // FILE *f = fopen("roms/Tron - Deadly Discs (1982)(Mattel).bin", "rb");
        FILE *f = fopen("roms/ExtendedBasic.bin", "rb");
        if (f == NULL) {
            perror("gamerom");
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

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (renderer == NULL) {
        printf("SDL_CreateRenderer Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 352, 224);

    SDL_AddTimer(16, timer_callback, NULL);
    SDL_Event event;
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
                frame_busy = false;

                uint32_t *pixels;
                int       pitch;
                SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);

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

                // 13.8 ms active state
                Z80ExecuteTStates(&z80context, 48302);

                // 2.8 ms v-sync
                is_vsync = true;
                Z80ExecuteTStates(&z80context, 9800);
                is_vsync = false;

                break;
            }
        }
    }
    return 0;
}
