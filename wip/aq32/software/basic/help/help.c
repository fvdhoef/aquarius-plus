#include "help.h"
#include "editor/screen.h"
#include "basic/common/buffers.h"
#include "editor/dialog.h"
#include "ctype2.h"

#define HELP_FILE    "/cores/aq32/basic.hlp"
#define HELP_ROWS    (TEXT_ROWS - 2)
#define HISTORY_SIZE 20

struct help_state {
    uint8_t *p_index;
    uint8_t *p_index_end;
    uint8_t *p_content;
    uint8_t *p_content_end;

    char window_title[80];
    int  cursor_line;
    int  cursor_pos;
    int  first_line;
    int  num_lines;
    int  current_topic_offset;
};

static int history[HISTORY_SIZE];
static int history_idx = -1;

static struct help_state state;

static bool init(void) {
    buf_reinit();
    memset(&state, 0, sizeof(state));

    unsigned sz   = 32768;
    state.p_index = buf_malloc(sz);
    if (!state.p_index)
        return false;
    state.p_index_end = state.p_index + sz;

    state.p_content = buf_malloc(sz);
    if (!state.p_content)
        return false;
    state.p_content_end = state.p_content + sz;

    state.current_topic_offset = -1;

    return true;
}

static int find_topic(const char *topic) {
    int topic_len = strlen(topic);

    FILE *f = fopen(HELP_FILE, "rb");
    if (f == NULL)
        return -1;

    // Find topic offset
    int offset = -1;

    unsigned index_max_sz = state.p_index_end - state.p_index;
    uint8_t  hdr[4];
    uint16_t index_data_len;
    if (fread(hdr, sizeof(hdr), 1, f) != 1 || memcmp(hdr, "HELP", 4) != 0 ||
        fread(&index_data_len, sizeof(index_data_len), 1, f) != 1 || index_data_len > index_max_sz ||
        fread(state.p_index, index_data_len, 1, f) != 1)
        goto done;

    uint8_t *p           = state.p_index;
    uint16_t num_entries = read_u16(p);
    p += 2;

    for (unsigned i = 0; i < num_entries; i++) {
        uint8_t *p_next = p + 1 + p[0] + 4;
        if (p[0] == topic_len && strncasecmp((const char *)p + 1, topic, topic_len) == 0) {
            offset = read_u32(p + 1 + p[0]);
            break;
        }
        p = p_next;
    }
    if (offset < 0)
        goto done;

done:
    fclose(f);
    return offset;
}

static bool load_topic(int offset) {
    if (offset < 0 || state.current_topic_offset == offset)
        return false;

    FILE *f = fopen(HELP_FILE, "rb");
    if (f == NULL)
        return false;

    // Skip past topics
    unsigned index_max_sz = state.p_index_end - state.p_index;
    uint8_t  hdr[4];
    uint16_t index_data_len;
    if (fread(hdr, sizeof(hdr), 1, f) != 1 || memcmp(hdr, "HELP", 4) != 0 ||
        fread(&index_data_len, sizeof(index_data_len), 1, f) != 1 || index_data_len > index_max_sz ||
        fseek(f, index_data_len, SEEK_CUR) != 0)
        goto error;

    // Load topic contents
    fseek(f, offset, SEEK_CUR);
    {
        state.cursor_line       = 0;
        state.cursor_pos        = 0;
        state.first_line        = 0;
        state.num_lines         = 2;
        uint8_t    *p           = state.p_content;
        const char *top_line    = "  <Back|_back>  <Contents|_toc>  <Index|_index>";
        int         top_line_sz = strlen(top_line);
        *(p++)                  = top_line_sz;
        for (int i = 0; i < top_line_sz; i++)
            *(p++) = top_line[i];
        *(p++) = 78;
        for (int i = 0; i < 78; i++)
            *(p++) = 25;

        unsigned content_max_sz = state.p_content_end - p;
        uint16_t data_len;
        uint16_t num_lines;
        if (fread(&data_len, sizeof(data_len), 1, f) != 1 || data_len > content_max_sz ||
            fread(&num_lines, sizeof(num_lines), 1, f) != 1)
            goto error;
        data_len -= 2;

        // Read page title
        uint8_t title_len = fgetc(f);
        char    tmp[74];
        if (title_len > sizeof(tmp) - 1)
            goto error;
        fread(tmp, title_len, 1, f);
        tmp[title_len] = 0;
        data_len -= 1 + title_len;
        num_lines--;
        snprintf(state.window_title, sizeof(state.window_title), "Help: %s", tmp);

        // Read remaining lines
        if (fread(p, data_len, 1, f) != 1)
            goto error;

        state.num_lines += num_lines;
    }
    fclose(f);
    state.current_topic_offset = offset;

    return true;

error:
    fclose(f);
    return false;
}

static bool navigate_back(void) {
    if (history_idx < 1)
        return false;

    int offset = history[--history_idx];
    return load_topic(offset);
}

static bool navigate_to_topic(const char *topic) {
    if (strcmp(topic, "_back") == 0) {
        return navigate_back();
    }

    int offset = find_topic(topic);
    if (offset < 0)
        return false;

    bool ok = load_topic(offset);

    // Record offset in history table
    if (ok) {
        if (history_idx == HISTORY_SIZE - 1) {
            for (int i = 0; i < HISTORY_SIZE - 1; i++) {
                history[i] = history[i + 1];
            }
        } else {
            history_idx++;
        }
        history[history_idx] = offset;
    }

    return ok;
}

static void draw_screen(void) {
    scr_draw_border(0, 0, TEXT_COLUMNS, TEXT_ROWS, COLOR_HELP, BORDER_FLAG_NO_SHADOW | BORDER_FLAG_TITLE_INVERSE, state.window_title);

    const uint8_t *p               = state.p_content;
    int            lines_remaining = state.num_lines;

    // Find first line
    for (int i = 0; i < state.first_line; i++) {
        p += 1 + p[0];
        lines_remaining--;
    }

    int cur_line = state.first_line;

    for (int row = 0; row < HELP_ROWS; row++) {
        cur_line = state.first_line + row;

        scr_locate(1 + row, 1);
        scr_setcolor(COLOR_HELP);

        int cur_pos = 0;

        if (lines_remaining > 0) {
            int line_len = p[0];
            p++;
            const uint8_t *p_next = p + line_len;

            bool bold      = false;
            bool escape    = false;
            int  hyperlink = 0;

            for (int i = 0; i < line_len; i++) {
                uint8_t val = *(p++);

                if (escape) {
                    escape = false;
                } else if (val == '\\') {
                    escape = true;
                    continue;
                } else if (val == '*') {
                    bold = !bold;
                    continue;
                } else if (val == '@') {
                    // Bulletpoint
                    val = 136;
                } else if (val == '<') {
                    // Link start
                    hyperlink = 1;
                    continue;
                } else if (hyperlink > 0 && val == '|') {
                    hyperlink = 2;
                    continue;
                } else if (hyperlink > 0 && val == '>') {
                    hyperlink = 0;
                    continue;
                } else if (hyperlink == 2) {
                    continue;
                }

                if (cur_line == state.cursor_line && cur_pos == state.cursor_pos) {
                    scr_setcolor(COLOR_CURSOR);
                } else if (hyperlink) {
                    scr_setcolor(COLOR_HELP_LINK);
                } else if (bold) {
                    scr_setcolor(COLOR_HELP_BOLD);
                } else {
                    scr_setcolor(COLOR_HELP);
                }

                scr_putchar(val);
                cur_pos++;
            }

            p = p_next;
            lines_remaining--;
        }

        while (cur_pos < 78) {
            if (cur_line == state.cursor_line && cur_pos == state.cursor_pos) {
                scr_setcolor(COLOR_CURSOR);
            } else {
                scr_setcolor(COLOR_HELP);
            }

            scr_putchar(' ');
            cur_pos++;
        }
    }
}

static void navigate(bool f1_pressed) {
    if (state.cursor_line < 0 || state.cursor_line >= state.num_lines ||
        state.cursor_pos < 0 || state.cursor_pos >= 78)
        return;

    // Find line cursor is on
    const uint8_t *p = state.p_content;
    for (int i = 0; i < state.cursor_line; i++)
        p += 1 + p[0];

    int line_len = p[0];
    p++;

    bool bold      = false;
    bool escape    = false;
    int  hyperlink = 0;
    int  cur_pos   = 0;
    int  first_pos = -1;
    int  last_pos  = -1;
    char topic[80];
    int  topic_len = 0;

    //
    for (int i = 0; i < line_len; i++) {
        uint8_t val = *(p++);

        if (escape) {
            escape = false;
        } else if (val == '\\') {
            escape = true;
            continue;

        } else if (val == '*') {
            bold = !bold;
            continue;

        } else if (val == '@') {
            // Bulletpoint
            val = 136;

        } else if (val == '<') {
            // Link start
            hyperlink = 1;
            first_pos = cur_pos;
            continue;

        } else if (hyperlink > 0 && val == '|') {
            hyperlink = 2;
            topic_len = 0;
            continue;

        } else if (hyperlink > 0 && val == '>') {
            if (topic_len >= 0 && state.cursor_pos >= first_pos && state.cursor_pos <= last_pos) {
                topic[topic_len] = 0;
                navigate_to_topic(topic);
                return;
            }
            hyperlink = 0;
            topic_len = 0;
            first_pos = -1;
            last_pos  = -1;
            continue;

        } else if (hyperlink == 2) {
            topic[topic_len++] = val;
            continue;
        }

        if (is_alpha(val) || val == '$' || hyperlink != 0) {
            if (first_pos < 0)
                first_pos = cur_pos;
            last_pos = cur_pos;

            topic[topic_len++] = val;

        } else if (hyperlink == 0) {
            if (topic_len >= 0 && state.cursor_pos >= first_pos && state.cursor_pos <= last_pos) {
                topic[topic_len] = 0;
                if (f1_pressed)
                    navigate_to_topic(topic);
                return;
            }

            first_pos = -1;
            last_pos  = -1;
            topic_len = 0;
        }
        cur_pos++;
    }

    if (topic_len >= 0 && state.cursor_pos >= first_pos && state.cursor_pos <= last_pos) {
        topic[topic_len] = 0;
        if (f1_pressed)
            navigate_to_topic(topic);
        return;
    }
}

static void help_loop(void) {
    while (1) {
        draw_screen();

        int key;
        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return;
        } else {
            uint8_t ch = key & 0xFF;
            switch (ch) {
                case CH_UP: state.cursor_line--; break;
                case CH_DOWN: state.cursor_line++; break;
                case CH_LEFT: state.cursor_pos--; break;
                case CH_RIGHT: state.cursor_pos++; break;
                case CH_HOME: {
                    if (key & KEY_MOD_CTRL)
                        state.cursor_line = 0;
                    state.cursor_pos = 0;
                    break;
                }
                case CH_END: {
                    if (key & KEY_MOD_CTRL)
                        state.cursor_line = state.num_lines;
                    state.cursor_pos = 77;
                    break;
                }
                case CH_PAGEUP: state.cursor_line -= HELP_ROWS - 1; break;
                case CH_PAGEDOWN: state.cursor_line += HELP_ROWS - 1; break;
                case CH_ENTER: navigate(false); break;
                case CH_F1:
                    if (key & KEY_MOD_ALT)
                        navigate_back();
                    else
                        navigate(true);
                    break;
            }

            state.cursor_pos  = clamp(state.cursor_pos, 0, 77);
            state.cursor_line = clamp(state.cursor_line, 0, state.num_lines);
            state.first_line  = clamp(state.first_line, max(0, state.cursor_line - (HELP_ROWS - 1)), state.cursor_line);
        }
    }
}

void help(const char *topic) {
    if (!init())
        return;
    if (!navigate_to_topic(topic))
        return;

    help_loop();
}

void help_reopen(void) {
    if (history_idx < 0)
        return;

    if (!init())
        return;
    load_topic(history[history_idx]);
    help_loop();
}
