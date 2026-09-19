#include "controls.h"
#include "screen.h"

void edit_field_reset(struct edit_field_ctx *ctx) {
    ctx->cursor_pos = strlen(ctx->buf);
    ctx->first_pos  = 0;
}

void edit_field_init(struct edit_field_ctx *ctx, int y, int x, int w, char *buf, size_t buf_len) {
    ctx->y       = y;
    ctx->x       = x;
    ctx->w       = w;
    ctx->buf     = buf;
    ctx->buf_len = buf_len;
    edit_field_reset(ctx);
}

void edit_field_draw(struct edit_field_ctx *ctx, bool show_cursor) {
    int line_len    = strlen(ctx->buf);
    ctx->cursor_pos = clamp(ctx->cursor_pos, 0, line_len);
    ctx->first_pos  = clamp(ctx->first_pos, ctx->cursor_pos - ctx->w + 1, max(0, ctx->cursor_pos - (ctx->w - 1)));
    line_len        = clamp(line_len - ctx->first_pos, 0, ctx->w);

    scr_locate(ctx->y, ctx->x);
    scr_setcolor(COLOR_FIELD);

    const char *ps = ctx->buf + ctx->first_pos;

    for (int i = 0; i < ctx->w; i++) {
        scr_setcolor(show_cursor && i == ctx->cursor_pos - ctx->first_pos ? COLOR_CURSOR : COLOR_FIELD);

        if (i < line_len) {
            scr_putchar(*(ps++));
        } else {
            scr_putchar(' ');
        }
    }
}

int edit_field_handle(struct edit_field_ctx *ctx) {
    while (1) {
        edit_field_draw(ctx, true);

        int key;
        while ((key = KEYBUF) < 0);
        if (key & KEY_IS_SCANCODE) {
            return key;

        } else {
            uint8_t ch = key & 0xFF;
            switch (ch) {
                case CH_LEFT: ctx->cursor_pos--; break;
                case CH_RIGHT: ctx->cursor_pos++; break;
                case CH_HOME: ctx->cursor_pos = 0; break;
                case CH_END: ctx->cursor_pos = strlen(ctx->buf); break;
                case '\t':
                case '\r': return key;

                case CH_BACKSPACE:
                case CH_DELETE: {
                    int linesz = strlen(ctx->buf);
                    if (linesz <= 0)
                        break;

                    if (ch == CH_BACKSPACE) {
                        if (ctx->cursor_pos <= 0)
                            break;
                        ctx->cursor_pos--;
                    }

                    uint8_t *p_cursor = (uint8_t *)ctx->buf + ctx->cursor_pos;
                    memmove(p_cursor, p_cursor + 1, linesz - ctx->cursor_pos);

                    break;
                }

                default: {
                    if (ch < ' ' || ch > '~')
                        break;

                    int linesz = strlen(ctx->buf);
                    if (linesz + 1 >= (int)ctx->buf_len)
                        break;

                    uint8_t *p_cursor = (uint8_t *)ctx->buf + ctx->cursor_pos;
                    memmove(p_cursor + 1, p_cursor, linesz - ctx->cursor_pos + 1);
                    *p_cursor = ch;
                    ctx->cursor_pos++;
                }
            }
        }
    }
}
