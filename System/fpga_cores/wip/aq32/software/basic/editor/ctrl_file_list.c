#include "controls.h"
#include "screen.h"

#define OPENDIREXT_FLAGS (DE_FLAG_DOTDOT | DE_FLAG_HIDDEN)

static void set_listing_color(int y, int x, int w, uint8_t col) {
    scr_setcolor(y == 6 ? COLOR_MENU_SEL : COLOR_MENU);

    scr_locate(y, x);
    while (w--)
        scr_putcolor(col);
}

static void draw_listing_line(int y, int x, int w, const struct esp_stat *st, const char *filename) {
    scr_setcolor(COLOR_MENU);

    scr_locate(y, x);
    if (!filename) {
        scr_fillchar(' ', w);
        return;
    }

    char tmp[32];
    if (st->date == 0 && st->time == 0) {
        scr_fillchar(' ', 16);
        w -= 16;
    } else {
        w -= snprintf(
            tmp, sizeof(tmp),
            " %02u-%02u-%02u %02u:%02u ",
            ((st->date >> 9) + 80) % 100,
            (st->date >> 5) & 15,
            st->date & 31,
            (st->time >> 11) & 31,
            (st->time >> 5) & 63);
        scr_puttext(tmp);
    }

    if (st->attr & DE_ATTR_DIR) {
        w -= snprintf(tmp, sizeof(tmp), "<DIR> ");
    } else {
        if (st->size < 1024) {
            w -= snprintf(tmp, sizeof(tmp), "%4lu  ", st->size);
        } else if (st->size < 1024 * 1024) {
            w -= snprintf(tmp, sizeof(tmp), "%4luK ", st->size >> 10);
        } else {
            w -= snprintf(tmp, sizeof(tmp), "%4luM ", st->size >> 20);
        }
    }
    scr_puttext(tmp);

    w--;

    const char *p = filename;
    while (w-- > 0) {
        char ch = *p;
        if (ch) {
            scr_putchar(ch);
            p++;
        } else {
            scr_putchar(' ');
        }
    }
    scr_putchar(' ');
}

static void draw_listing_page(int y, int x, int w, int rows, int page) {
    struct esp_stat st;
    char            tmp[256];

    int dd = esp_opendirext("", OPENDIREXT_FLAGS, page * rows);
    for (int i = 0; i < rows; i++) {
        if (dd >= 0) {
            int res = esp_readdir(dd, &st, tmp, sizeof(tmp));
            if (res < 0) {
                esp_closedir(dd);
                dd = -1;
            }
        }

        draw_listing_line(y, x, w, &st, dd >= 0 ? tmp : NULL);
        y++;
    }
    if (dd >= 0)
        esp_closedir(dd);
}

static int get_num_entries(void) {
    struct esp_stat st;
    char            tmp[16];
    int             result = 0;

    int dd = esp_opendirext("", OPENDIREXT_FLAGS, 0);
    if (dd >= 0) {
        while (1) {
            if (esp_readdir(dd, &st, tmp, sizeof(tmp)) < 0)
                break;
            result++;
        }
        esp_closedir(dd);
    }
    return result;
}

void file_list_reset(struct file_list_ctx *ctx) {
    ctx->old_selection = INT32_MIN;
    ctx->selection     = 0;
    ctx->total         = -1;
}

void file_list_init(struct file_list_ctx *ctx, int y, int x, int w, int rows, char *fn_buf, size_t fn_bufsize) {
    ctx->y          = y;
    ctx->x          = x;
    ctx->w          = w;
    ctx->rows       = rows;
    ctx->fn_buf     = fn_buf;
    ctx->fn_bufsize = fn_bufsize;
    file_list_reset(ctx);
}

void file_list_draw(struct file_list_ctx *ctx, bool show_selection) {
    if (ctx->total < 0)
        ctx->total = get_num_entries();

    ctx->selection = clamp(ctx->selection, 0, ctx->total - 1);

    if (ctx->old_selection != ctx->selection) {
        if (ctx->old_selection / ctx->rows != ctx->selection / ctx->rows) {
            draw_listing_page(ctx->y, ctx->x, ctx->w, ctx->rows, ctx->selection / ctx->rows);
        }

        if (ctx->old_selection >= 0)
            set_listing_color(ctx->y + ctx->old_selection % ctx->rows, ctx->x, ctx->w, COLOR_MENU);
        ctx->old_selection = ctx->selection;
    }
    set_listing_color(ctx->y + ctx->selection % ctx->rows, ctx->x, ctx->w, show_selection ? COLOR_MENU_SEL : COLOR_MENU);
}

int file_list_handle(struct file_list_ctx *ctx) {
    while (1) {
        file_list_draw(ctx, true);

        int key;
        while ((key = KEYBUF) < 0);
        if (key & KEY_IS_SCANCODE) {
            return key;

        } else {
            uint8_t ch = key & 0xFF;
            switch (ch) {
                case CH_DOWN: ctx->selection++; break;
                case CH_UP: ctx->selection--; break;
                case CH_PAGEDOWN: ctx->selection += ctx->rows; break;
                case CH_PAGEUP: ctx->selection -= ctx->rows; break;
                case '\r': {
                    int dd = esp_opendirext("", OPENDIREXT_FLAGS, ctx->selection);
                    if (dd >= 0) {
                        int res = esp_readdir(dd, &ctx->st, ctx->fn_buf, ctx->fn_bufsize);
                        esp_closedir(dd);
                        if (res == 0)
                            return key;
                    }
                    break;
                }
                default: return key;
            }
        }
    }
}
