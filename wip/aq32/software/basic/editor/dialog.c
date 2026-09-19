#include "dialog.h"
#include "screen.h"
#include "esp.h"
#include "controls.h"

static void draw_current_dir(int y, int x, int w) {
    char tmp[256];
    getcwd((char *)tmp, sizeof(tmp));
    scr_locate(y, x);
    scr_setcolor(COLOR_MENU);
    scr_puttext(" Current dir: ");

    int path_len = strlen(tmp);
    if (path_len > w - 14) {
        scr_puttext("...");
        scr_puttext(tmp + path_len - (w - 3 - 14));
    } else {
        scr_puttext(tmp);
        scr_fillchar(' ', w - 14 - path_len);
    }
}

bool dialog_open(char *fn_buf, size_t fn_bufsize) {
    struct file_list_ctx flctx;

    int w = TEXT_COLUMNS - 12;
    int h = TEXT_ROWS - 6;
    int x = (TEXT_COLUMNS - w) / 2;
    int y = (TEXT_ROWS - h) / 2;

    scr_draw_border(y, x, w, h, COLOR_MENU, 0, "Open");
    scr_draw_separator(y + 2, x, w, COLOR_MENU);
    scr_status_msg("Enter=Select   Esc=Cancel   Up/Down/PgUp/PgDn=Change Selection");

    draw_current_dir(y + 1, x + 1, w - 2);
    file_list_init(&flctx, y + 3, x + 1, w - 2, h - 4, fn_buf, fn_bufsize);

    while (1) {
        int key = file_list_handle(&flctx);
        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return false;
        } else {
            uint8_t ch = key & 0xFF;
            if (ch == '\r') {
                if (flctx.st.attr & DE_ATTR_DIR) {
                    esp_chdir(fn_buf);
                    draw_current_dir(y + 1, x + 1, w - 2);
                    file_list_reset(&flctx);
                } else {
                    return true;
                }
            }
        }
    }
    return false;
}

static bool normalize_filename(char *fn_buf, size_t fn_bufsize) {
    bool changed_dir = false;

    // Replace all backslashes with forward slashes
    {
        char *p = fn_buf;
        while (*p) {
            if (*p == '\\')
                *p = '/';
            p++;
        }
    }

    // Resolve directory names
    while (1) {
        char *p = strchr(fn_buf, '/');
        if (p == NULL)
            break;

        *p = 0;
        esp_chdir(*fn_buf == 0 ? "/" : fn_buf);
        changed_dir = true;
        memmove(fn_buf, p + 1, strlen(p + 1) + 1);
    }

    // Change directory if remaining part is directory name
    struct esp_stat st;
    if (*fn_buf && esp_stat(fn_buf, &st) == 0 && (st.attr & DE_ATTR_DIR)) {
        esp_chdir(fn_buf);
        changed_dir = true;

        *fn_buf = 0;
    }

    return !changed_dir;
}

bool dialog_save(char *fn_buf, size_t fn_bufsize) {
    struct file_list_ctx  flctx;
    struct edit_field_ctx efctx;

    int w = TEXT_COLUMNS - 12;
    int h = TEXT_ROWS - 6;
    int x = (TEXT_COLUMNS - w) / 2;
    int y = (TEXT_ROWS - h) / 2;

    scr_draw_border(y, x, w, h, COLOR_MENU, 0, "Save");
    scr_draw_separator(y + 2, x, w, COLOR_MENU);
    scr_draw_separator(y + h - 3, x, w, COLOR_MENU);
    scr_status_msg("Enter=Select   Esc=Cancel   Up/Down/PgUp/PgDn/Tab=Change Selection");

    draw_current_dir(y + 1, x + 1, w - 2);
    scr_locate(y + h - 2, x + 1);
    scr_puttext(" Filename: ");

    char tmp[256];
    file_list_init(&flctx, y + 3, x + 1, w - 2, h - 6, tmp, sizeof(tmp));
    file_list_draw(&flctx, false);

    edit_field_init(&efctx, y + h - 2, x + 12, w - 14, fn_buf, fn_bufsize);
    edit_field_draw(&efctx, false);

    scr_setcolor(COLOR_MENU);
    scr_putchar(' ');

    int cur_field = 1;

    while (1) {
        int key;
        if (cur_field == 0) {
            key = file_list_handle(&flctx);
        } else if (cur_field == 1) {
            key = edit_field_handle(&efctx);
        }

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return false;
        } else {
            uint8_t ch = key & 0xFF;
            if (ch == '\r') {
                if (cur_field == 0) {
                    if (flctx.st.attr & DE_ATTR_DIR) {
                        esp_chdir(tmp);
                        draw_current_dir(y + 1, x + 1, w - 2);
                        file_list_reset(&flctx);
                    } else {
                        snprintf(fn_buf, fn_bufsize, "%s", tmp);
                        edit_field_reset(&efctx);
                        file_list_draw(&flctx, false);
                        cur_field = 1;
                    }

                } else if (cur_field == 1) {
                    if (normalize_filename(fn_buf, fn_bufsize) && strlen(fn_buf) > 0)
                        return true;

                    draw_current_dir(y + 1, x + 1, w - 2);
                    file_list_reset(&flctx);
                    file_list_draw(&flctx, false);
                }

            } else if (ch == '\t') {
                if (cur_field == 0) {
                    file_list_draw(&flctx, false);
                    cur_field = 1;
                } else if (cur_field == 1) {
                    edit_field_draw(&efctx, false);
                    cur_field = 0;
                }
            }
        }
    }
    return false;
}

int dialog_confirm(const char *title, const char *text) {
    const char *choices_str = "<&Yes>   <&No>   <&Cancel>";

    scr_status_msg("Y=Yes   N=No   C/Esc=Cancel");

    int text_len    = strlen(text);
    int choices_len = strlen(choices_str);

    int w = max(text_len, choices_len) + 4;
    int h = 7;
    int x = (TEXT_COLUMNS - w) / 2;
    int y = (TEXT_ROWS - h) / 2;

    scr_draw_border(y, x, w, h, COLOR_MENU, 0, title);
    scr_setcolor(COLOR_MENU);
    scr_center_text(y + 1, x + 1, w - 2, "", false);
    scr_center_text(y + 2, x + 1, w - 2, text, false);
    scr_center_text(y + 3, x + 1, w - 2, "", false);
    scr_draw_separator(y + 4, x, w, COLOR_MENU);
    scr_center_text(y + 5, x + 1, w - 2, choices_str, true);

    while (1) {
        int key;
        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return -1;
        } else {
            uint8_t ch = key & 0xFF;
            switch (toupper(ch)) {
                case 'Y': return 1;
                case 'N': return 0;
                case 'C': return -1;
            }
        }
    }
}

void dialog_message(const char *title, const char *text) {
    scr_status_msg("Press enter or escape to dismiss message.");

    int w = max(strlen(text), strlen(title)) + 4;
    int h = 5;
    int x = (TEXT_COLUMNS - w) / 2;
    int y = (TEXT_ROWS - h) / 2;

    scr_draw_border(y, x, w, h, COLOR_MENU, 0, title);
    scr_setcolor(COLOR_MENU);
    scr_center_text(y + 1, x + 1, w - 2, "", false);
    scr_center_text(y + 2, x + 1, w - 2, text, false);
    scr_center_text(y + 3, x + 1, w - 2, "", false);

    while (1) {
        int key;
        while ((key = KEYBUF) < 0);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return;
        } else {
            uint8_t ch = key & 0xFF;
            switch (toupper(ch)) {
                case CH_ENTER: return;
            }
        }
    }
}

bool dialog_edit_field(const char *title, char *buf, size_t buf_size) {
    scr_status_msg("Enter=Confirm   Esc=Cancel");

    int w = 70;
    int h = 5;
    int x = (TEXT_COLUMNS - w) / 2;
    int y = (TEXT_ROWS - h) / 2;

    scr_draw_border(y, x, w, h, COLOR_MENU, 0, title);
    scr_setcolor(COLOR_MENU);
    scr_center_text(y + 1, x + 1, w - 2, "", false);

    scr_setcolor(COLOR_MENU);
    scr_locate(y + 2, x + 1);
    scr_putchar(' ');
    struct edit_field_ctx efctx;
    edit_field_init(&efctx, y + 2, x + 2, w - 4, buf, buf_size);
    edit_field_draw(&efctx, true);
    scr_setcolor(COLOR_MENU);
    scr_putchar(' ');

    // scr_center_text(y + 2, x + 1, w - 2, text, false);
    scr_setcolor(COLOR_MENU);
    scr_center_text(y + 3, x + 1, w - 2, "", false);

    while (1) {
        int key;
        key = edit_field_handle(&efctx);

        if (key & KEY_IS_SCANCODE) {
            uint8_t scancode = key & 0xFF;
            // Escape?
            if (((key & KEY_KEYDOWN) && scancode == SCANCODE_ESC))
                return false;
        } else {
            uint8_t ch = key & 0xFF;
            switch (toupper(ch)) {
                case CH_ENTER: return true;
            }
        }
    }
}
