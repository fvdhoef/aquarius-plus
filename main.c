#include "common.h"
#include <SDL.h>
#undef main

#include "audio.h"
#include "z80.h"
#include "ay8910.h"
#include "ch376.h"
#include "direnum.h"

#define AUDIO_LEVEL (16000)

extern unsigned char charrom_bin[2048]; // Character ROM contents

struct emulation_state {
    Z80Context    z80context;       // Z80 emulation core state
    int           line_hcycles;     // Half-cycles for this line
    int           sample_hcycles;   // Half-cycles for this sample
    int           linenr;           // Current display line
    int16_t       audio_out;        // Audio level of internal sound output (also used for cassette interface)
    bool          cpm_remap;        // Remap memory for CP/M
    uint8_t       scramble_value;   // Value to XOR (scramble) the external data bus with
    uint8_t       keyb_matrix[8];   // Keyboard matrix (8 x 6bits)
    bool          expander_enabled; // Mini-expander enabled?
    uint8_t       ay_addr;          // Mini-expander - AY-3-8910: Selected address to access via data register
    uint8_t       handctrl1;        // Mini-expander - Hand controller 1 state (connected to port 1 of AY-3-8910)
    uint8_t       handctrl2;        // Mini-expander - Hand controller 2 state (connected to port 1 of AY-3-8910)
    bool          ramexp_enabled;   // RAM expansion enabled?
    uint8_t       rom[0x2000];      // 0x0000-0x1FFF: System ROM
    uint8_t       ram[0x1000];      // 0x3000-0x3FFF: System RAM
    uint8_t       ramexp[0x8000];   // 0x4000-0xBFFF: RAM expansion
    uint8_t       gamerom[0x4000];  // 0xC000-0xFFFF: Cartridge
    struct ay8910 ay_state;         // AY-3-8910 emulation state
};

static struct emulation_state state = {
    .audio_out        = AUDIO_LEVEL,
    .expander_enabled = true,
    .keyb_matrix      = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
    .handctrl1        = 0xFF,
    .handctrl2        = 0xFF,
    .ramexp_enabled   = true,
};

// Aquarius palette
static const uint32_t palette[16] = {
    0x101010, 0xf71010, 0x10f710, 0xf7ef10,
    0x2121de, 0xf710f7, 0x31c6c6, 0xf7f7f7,
    0xc6c6c6, 0x29adad, 0xc621c6, 0x42108c,
    0xf7f773, 0x10F710, 0x21ce42, 0x313131};

static uint8_t mem_read(size_t param, uint16_t addr) {
    (void)param;
    if (state.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    uint8_t result = 0xFF;
    if (addr < 0x2000) {
        result = state.rom[addr];
    } else if (addr >= 0x3000 && addr < 0x4000) {
        result = state.ram[addr - 0x3000];
    } else if (state.ramexp_enabled && addr >= 0x4000 && addr < 0xC000) {
        result = state.ramexp[addr - 0x4000];
    } else if (addr >= 0xC000) {
        result = state.gamerom[addr - 0xC000] ^ state.scramble_value;
    } else {
        printf("mem_read(0x%04x) -> 0x%02x\n", addr, result);
    }
    return result;
}

static void mem_write(size_t param, uint16_t addr, uint8_t data) {
    (void)param;
    if (state.cpm_remap) {
        if (addr < 0x4000)
            addr += 0xC000;
        if (addr >= 0xC000)
            addr -= 0xC000;
    }

    if (addr >= 0x3000 && addr < 0x4000) {
        state.ram[addr - 0x3000] = data;
    } else if (state.ramexp_enabled && addr >= 0x4000 && addr < 0xC000) {
        state.ramexp[addr - 0x4000] = data;
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
            if (state.expander_enabled) {
                // AY-3-8910 register read
                switch (state.ay_addr) {
                    case 14: result = state.handctrl1; break;
                    case 15: result = state.handctrl2; break;
                    default:
                        if (state.ay_addr < 14)
                            result = ay8910_read_reg(&state.ay_state, state.ay_addr);

                        // printf("AY-3-8910 R%u => 0x%02X\n", state.ay_addr, result);
                        break;
                }
            }
            break;

        case 0xFC: printf("Cassette port input (%04x) -> %02x\n", addr, result); break;
        case 0xFD: result = (state.linenr >= 224) ? 0 : 1; break;
        case 0xFE: printf("Clear to send status (%04x) -> %02x\n", addr, result); break;
        case 0xFF: {
            // Keyboard matrix. Selected rows are passed in the upper 8 address lines.
            uint8_t rows = addr >> 8;

            // Wire-AND all selected rows.
            result = 0xFF;
            for (int i = 0; i < 8; i++) {
                if ((rows & (1 << i)) == 0) {
                    result &= state.keyb_matrix[i];
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
            if (state.expander_enabled) {
                // AY-3-8910 register write
                if (state.ay_addr < 14)
                    ay8910_write_reg(&state.ay_state, state.ay_addr, data);

                // printf("AY-3-8910 R%u=0x%02X\n", state.ay_addr, data);
            }
            break;
        case 0xF7:
            if (state.expander_enabled) {
                // AY-3-8910 address latch
                state.ay_addr = data;
            }
            break;

        case 0xFC: state.audio_out = (data & 1) ? AUDIO_LEVEL : -AUDIO_LEVEL; break;
        case 0xFD: state.cpm_remap = (data & 1) != 0; break;
        case 0xFE: printf("1200 bps serial printer (%04x) = %u\n", addr, data & 1); break;
        case 0xFF: state.scramble_value = data; break;
        default: printf("io_write(0x%02x, 0x%02x)\n", addr & 0xFF, data); break;
    }
}

static void reset(void) {
    Z80RESET(&state.z80context);
    state.z80context.ioRead   = io_read;
    state.z80context.ioWrite  = io_write;
    state.z80context.memRead  = mem_read;
    state.z80context.memWrite = mem_write;

    state.scramble_value = 0;
    state.cpm_remap      = false;

    ay8910_reset(&state.ay_state);
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

    state.handctrl1 = 0xFF;
    switch (handctrl_pressed & 0xF) {
        case LEFT: state.handctrl1 &= ~(1 << 3); break;
        case UP | LEFT: state.handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
        case UP: state.handctrl1 &= ~(1 << 2); break;
        case UP | RIGHT: state.handctrl1 &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
        case RIGHT: state.handctrl1 &= ~(1 << 1); break;
        case DOWN | RIGHT: state.handctrl1 &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
        case DOWN: state.handctrl1 &= ~(1 << 0); break;
        case DOWN | LEFT: state.handctrl1 &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
        default: break;
    }
    if (handctrl_pressed & K1)
        state.handctrl1 &= ~(1 << 6);
    if (handctrl_pressed & K2)
        state.handctrl1 &= ~((1 << 7) | (1 << 2));
    if (handctrl_pressed & K3)
        state.handctrl1 &= ~((1 << 7) | (1 << 5));
    if (handctrl_pressed & K4)
        state.handctrl1 &= ~(1 << 5);
    if (handctrl_pressed & K5)
        state.handctrl1 &= ~((1 << 7) | (1 << 1));
    if (handctrl_pressed & K6)
        state.handctrl1 &= ~((1 << 7) | (1 << 0));

    if (key < 0) {
        return;
    }

    if (keydown) {
        state.keyb_matrix[key / 6] &= ~(1 << (key % 6));
    } else {
        state.keyb_matrix[key / 6] |= (1 << (key % 6));
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
    uint8_t border_ch    = state.ram[0];
    uint8_t border_color = state.ram[0x400];

    for (int row = 0; row < 28; row++) {
        for (int column = 0; column < 44; column++) {
            if (row >= 2 && row < 26 && column >= 2 && column < 42)
                continue;

            draw_char(pixels, pitch, border_ch, border_color, row, column);
        }
    }

    for (int row = 0; row < 24; row++) {
        for (int column = 0; column < 40; column++) {
            uint8_t ch    = state.ram[(row + 1) * 40 + column];
            uint8_t color = state.ram[0x400 + (row + 1) * 40 + column];
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

// 3579545 Hz -> 59659 cycles / frame
// 7159090 Hz -> 119318 cycles / frame

// 455x262=119210 -> 60.05 Hz
// 51.2us + 1.5us + 4.7us + 6.2us = 63.6 us
// 366 active pixels

#define HCYCLES_PER_LINE (455)
#define HCYCLES_PER_SAMPLE (149)

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
            state.z80context.tstates = 0;
            Z80Execute(&state.z80context);
            int delta = state.z80context.tstates * 2;

            state.line_hcycles += delta;
            state.sample_hcycles += delta;

            if (state.line_hcycles >= HCYCLES_PER_LINE) {
                state.line_hcycles -= HCYCLES_PER_LINE;

                state.linenr++;
                if (state.linenr == 262) {
                    render_screen(renderer);
                    state.linenr = 0;
                }
            }
        } while (state.sample_hcycles < HCYCLES_PER_SAMPLE);

        state.sample_hcycles -= HCYCLES_PER_SAMPLE;

        // Take average of 5 AY8910 samples to match sampling rate (16*5*44100 = 3.528MHz)
        float samples = 0;
        for (int i = 0; i < 5; i++) {
            samples += ay8910_render(&state.ay_state);
        }
        samples /= 5.0f;

        abuf[aidx] = state.audio_out + (uint16_t)(samples * AUDIO_LEVEL);
    }

    // Return buffer to audio subsystem.
    audio_put_buffer(abuf);
}

int main(int argc, char *argv[]) {
    char *base_path = SDL_GetBasePath();

    direnum_ctx_t dec = direnum_open(".");
    if (dec != NULL) {
        struct direnum_ent dee;
        while (direnum_read(dec, &dee)) {
            printf("%s %u %u %lu\n", dee.filename, dee.size, dee.attr, dee.t);
        }
        direnum_close(dec);
    }

    char rom_path[1024];
    snprintf(rom_path, sizeof(rom_path), "%s%s", base_path, "aquarius.rom");

    char cartrom_path[1024];
    cartrom_path[0] = 0;

    int  opt;
    bool params_ok = true;
    while ((opt = getopt(argc, argv, "r:c:XR")) != -1) {
        switch (opt) {
            case 'r': snprintf(rom_path, sizeof(rom_path), "%s", optarg); break;
            case 'c': snprintf(cartrom_path, sizeof(cartrom_path), "%s", optarg); break;
            case 'X': state.expander_enabled = false; break;
            case 'R': state.ramexp_enabled = false; break;
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
        if (fread(state.rom, sizeof(state.rom), 1, f) != 1) {
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
            if (fread(state.gamerom + 8192, filesize, 1, f) <= 0) {
                fprintf(stderr, "Error during reading of cartridge ROM image.\n");
                exit(1);
            }
            // Mirror ROM to $C000
            memcpy(state.gamerom, state.gamerom + 8192, 8192);

        } else if (filesize == 16384) {
            if (fread(state.gamerom, filesize, 1, f) <= 0) {
                fprintf(stderr, "Error during reading of cartridge ROM image.\n");
                exit(1);
            }

        } else {
            fprintf(stderr, "Invalid cartridge ROM file: %u, should be either exactly 8 or 16KB.\n", filesize);
        }

        fclose(f);
    }

    unsigned long wnd_width = 352 * 2, wnd_height = 224 * 2;
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
    ay8910_init(&state.ay_state);
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
