#include "state.h"
#include "ctype2.h"
#include "scr_code/edit_ops.h"

static const char *hexlut = "0123456789abcdef";

edit_state_t edit_state = {
    .editing = false,
    .mode    = MODE_CODE,

    .sfx_edit = {
        .sfx_idx    = 0,
        .octave     = 2,
        .volume     = 5,
        .waveform   = 0,
        .effect     = 0,
        .cursor_row = 0,
        .cursor_col = 0,
    },

    .spr_edit = {
        .color = 6,
    },
};

data_state_t data_state = {};
game_state_t game_state;

#include "scr_console/console.h"

enum {
    LOADMODE_NONE,
    LOADMODE_GFX,
    LOADMODE_GFF,
    LOADMODE_MAP,
    LOADMODE_SFX,
    LOADMODE_MUSIC,
    LOADMODE_LUA,
};

static unsigned get_nibble(char ch) {
    ch = to_lower(ch);
    if (ch >= '0' && ch <= '9')
        return ch - '0';
    if (ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return 0;
}

int state_load_cart(const char *path) {
    char line[260];

    unsigned t0 = frame_cnt;

    FILE *f = fopen(path, "r");
    if (!f) {
        return -1;
    }
    if (fgets(line, sizeof(line), f) == NULL || strncmp(line, "aqua-8 cartridge", 16) != 0) {
        fclose(f);
        return 0;
    }

    code_edit_reset_state();
    editbuf_t *eb = edit_state.code_edit.editbuf;
    memset(&data_state, 0, sizeof(data_state));

    uint8_t       *p_spr     = (uint8_t *)data_state.sprites;
    const uint8_t *p_spr_end = p_spr + sizeof(data_state.sprites);
    unsigned       sfx_idx   = 0;
    uint8_t       *p_code    = eb->p_buf;

    unsigned mode = LOADMODE_NONE;

    while (1) {
        if (fgets(line, sizeof(line), f) == NULL)
            break;

        unsigned len      = strlen(line);
        unsigned old_mode = mode;

        if (strcmp(line, "__gfx__\n") == 0)
            mode = LOADMODE_GFX;
        else if (strcmp(line, "__gff__\n") == 0)
            mode = LOADMODE_GFF;
        else if (strcmp(line, "__map__\n") == 0)
            mode = LOADMODE_MAP;
        else if (strcmp(line, "__sfx__\n") == 0)
            mode = LOADMODE_SFX;
        else if (strcmp(line, "__music__\n") == 0)
            mode = LOADMODE_MUSIC;
        else if (strcmp(line, "__lua__\n") == 0)
            mode = LOADMODE_LUA;
        else {
            switch (mode) {
                case LOADMODE_GFX: {
                    if (len != 129)
                        break;

                    for (int i = 0; i < 128; i += 2) {
                        if (p_spr >= p_spr_end)
                            break;

                        *(p_spr++) = get_nibble(line[i + 0]) | (get_nibble(line[i + 1]) << 4);
                    }
                    break;
                }
                case LOADMODE_GFF: {
                    break;
                }
                case LOADMODE_MAP: {
                    break;
                }
                case LOADMODE_SFX: {
                    if (len != 169 || sfx_idx >= 64)
                        break;
                    sfx_t      *sfx = &data_state.sfx[sfx_idx++];
                    const char *ps  = line;

                    uint8_t editor_mode = get_nibble(*(ps++)) << 4;
                    editor_mode |= get_nibble(*(ps++));
                    sfx->speed = get_nibble(*(ps++)) << 4;
                    sfx->speed |= get_nibble(*(ps++));
                    sfx->loop_start = get_nibble(*(ps++)) << 4;
                    sfx->loop_start |= get_nibble(*(ps++));
                    sfx->loop_end = get_nibble(*(ps++)) << 4;
                    sfx->loop_end |= get_nibble(*(ps++));

                    for (int i = 0; i < 32; i++) {
                        uint8_t pitch = get_nibble(*(ps++)) << 4;
                        pitch |= get_nibble(*(ps++));
                        uint8_t wf    = get_nibble(*(ps++));
                        uint8_t vol   = get_nibble(*(ps++));
                        uint8_t fx    = get_nibble(*(ps++));
                        sfx->notes[i] = ((fx & 7) << 12) | ((vol & 7) << 9) | ((wf & 7) << 6) | (pitch & 63);
                    }
                    break;
                }
                case LOADMODE_MUSIC: {
                    break;
                }
                case LOADMODE_LUA: {
                    unsigned remaining = eb->p_buf_end - p_code;
                    if (remaining <= 1) {
                        mode = LOADMODE_NONE;
                        break;
                    }
                    if (len + 1 >= remaining) {
                        len = remaining - 1;
                    }
                    memcpy(p_code, line, len);
                    p_code += len;
                    break;
                }
            }
        }

        if (mode != old_mode) {
            const char *mode_str;
            switch (mode) {
                case LOADMODE_GFX: mode_str = "sprites"; break;
                case LOADMODE_GFF: mode_str = "sprite flags"; break;
                case LOADMODE_MAP: mode_str = "map data"; break;
                case LOADMODE_SFX: mode_str = "sound effects"; break;
                case LOADMODE_MUSIC: mode_str = "music"; break;
                case LOADMODE_LUA: mode_str = "code"; break;
                default: break;
            }
            if (mode != LOADMODE_NONE)
                console_printf("- Loading %s\r\n", mode_str);
        }

        // console_putline(line);
    }
    fclose(f);

    // Finalize code edit buffer
    {
        unsigned size   = p_code - eb->p_buf;
        uint8_t *p_load = eb->p_buf_end - size;
        memmove(p_load, eb->p_buf, size);
        if (!editbuf_normalize(eb, p_load, eb->p_buf_end)) {
            return -1;
        }
    }

    unsigned t = frame_cnt - t0;
    console_printf("%u frames\r\n", t);

    return editbuf_get_size(eb);
}

static int fwrite_hex_line(FILE *f, const void *buf, size_t len) {
    if (len > 128)
        return -1;

    char tmp[256 + 1];

    const uint8_t *ps = buf;
    char          *pd = tmp;
    for (unsigned i = 0; i < len; i++) {
        *(pd++) = hexlut[ps[0] & 0xF];
        *(pd++) = hexlut[(ps[0] >> 4) & 0xF];
        ps++;
    }
    *(pd++) = '\n';
    return fwrite(tmp, pd - tmp, 1, f);
}

int state_save_cart(const char *path) {
    unsigned t0 = frame_cnt;

    FILE *f = fopen(path, "w");
    if (!f) {
        return -1;
    }

    fputs("aqua-8 cartridge\n", f);
    fputs("version 1\n", f);

    // Graphics
    {
        // Save sprites
        {
            // Determine lines of sprite data to save
            int lines = 128;
            while (lines > 0) {
                bool empty = true;
                for (int i = 0; i < 16; i++) {
                    if (data_state.sprites[(lines - 1) * 16 + i])
                        empty = false;
                    break;
                }
                if (!empty)
                    break;
                lines--;
            }

            // Save sprite data
            if (lines > 0) {
                console_printf("- Saving sprites (%d lines)\r\n", lines);
                fputs("__gfx__\n", f);
                for (int j = 0; j < lines; j++) {
                    fwrite_hex_line(f, &data_state.sprites[j * 16], 64);
                }
            }
        }

        console_printf("- Saving sprite flags\r\n");
        fputs("__gff__\n", f);

        console_printf("- Saving map data\r\n");
        fputs("__map__\n", f);
    }

    // Sound
    {
        // Sound effect
        {
            // Determine number of sound effects to save
            int count = 64;
            while (count > 0) {
                if (!sfx_is_empty(&data_state.sfx[count - 1]))
                    break;
                count--;
            }

            if (count > 0) {
                console_printf("- Saving %d sound effects\r\n", count);
                fputs("__sfx__\n", f);
                for (int j = 0; j < count; j++) {
                    const sfx_t *sfx = &data_state.sfx[j];

                    char  buf[168 + 2];
                    char *pd = buf;

                    uint8_t editor_mode = 0;

                    *(pd++) = hexlut[editor_mode >> 4];
                    *(pd++) = hexlut[editor_mode & 0xF];
                    *(pd++) = hexlut[sfx->speed >> 4];
                    *(pd++) = hexlut[sfx->speed & 0xF];
                    *(pd++) = hexlut[sfx->loop_start >> 4];
                    *(pd++) = hexlut[sfx->loop_start & 0xF];
                    *(pd++) = hexlut[sfx->loop_end >> 4];
                    *(pd++) = hexlut[sfx->loop_end & 0xF];

                    for (int i = 0; i < 32; i++) {
                        uint16_t note_code = sfx->notes[i];
                        unsigned pitch     = note_code & 63;
                        unsigned wf        = (note_code >> 6) & 7;
                        unsigned vol       = (note_code >> 9) & 7;
                        unsigned fx        = (note_code >> 12) & 7;

                        *(pd++) = hexlut[pitch >> 4];
                        *(pd++) = hexlut[pitch & 0xF];
                        *(pd++) = hexlut[wf];
                        *(pd++) = hexlut[vol];
                        *(pd++) = hexlut[fx];
                    }

                    *(pd++) = '\n';
                    fwrite(buf, pd - buf, 1, f);
                }
            }
        }

        console_printf("- Saving music\r\n");
        fputs("__music__\n", f);
    }

    // Source code
    {
        const uint8_t *buf;
        unsigned       size = editbuf_get_buf(edit_state.code_edit.editbuf, &buf);
        if (size > 0) {
            console_printf("- Saving code\r\n");
            fputs("__lua__\n", f);
            fwrite(buf, size, 1, f);
            if (buf[size - 1] != '\n')
                fputc('\n', f);
        }
    }

    fclose(f);

    unsigned t = frame_cnt - t0;
    console_printf("%u frames\r\n", t);

    return 0;
}
