#include "scr.h"

static void on_tool_click(unsigned tool) {
    switch (tool) {
        case TOOL_ROTATE: break;
        case TOOL_HFLIP: break;
        case TOOL_VFLIP: break;
        case TOOL_DELETE: break;

        default:
            edit_state.spr_edit.tool = tool;
            break;
    }
}

const char *tool_status_text[] = {
    "Draw",                          // TOOL_PIXEL
    "Shape: line",                   // TOOL_LINE
    "Fill",                          // TOOL_FILL
    "Select",                        // TOOL_SELECT
    "Rotate",                        // TOOL_ROTATE
    "Color picker",                  // TOOL_COLORPICK
    "Shape: circle (Ctrl: Fill)",    // TOOL_CIRCLE
    "Shape: rectangle (Ctrl: Fill)", // TOOL_RECT
    "Stamp from clipboard",          // TOOL_STAMP
    "Flip horizontally",             // TOOL_HFLIP
    "Flip vertically",               // TOOL_VFLIP
    "Erase",                         // TOOL_DELETE
};

static void draw(void) {
    scr_common(5);

    int x, y;

    // draw_text("#0", 1, 8, 7, false);

    // Palette
    {
        x = 1;
        y = 8;

        rect_t r = {x, y, x + 1 + 64, y + 1 + 16};
        draw_rect(&r, 0);
        rect_shrink(&r, 1);

        if (mouse_hover(&r, MOUSE_SPR_HAND)) {
            unsigned hover_color = ((edit_state.mouse_ev.y - r.y0) / 8) * 8 +
                                   ((edit_state.mouse_ev.x - r.x0) / 8);

            snprintf(edit_state.status_text, sizeof(edit_state.status_text), "Color %u", hover_color);

            if (edit_state.mouse_ev.buttons == 1)
                edit_state.spr_edit.color = hover_color;
        }

        x += 1;
        y += 1;

        for (int i = 0; i < 16; i++) {
            int row = i / 8;
            int col = i % 8;

            fill_rect(&(rect_t){x + col * 8, y + row * 8, x + col * 8 + 7, y + row * 8 + 7}, i);
        }

        rect_t r2;
        r2.x0 = r.x0 + (edit_state.spr_edit.color % 8) * 8 - 1;
        r2.y0 = r.y0 + (edit_state.spr_edit.color / 8) * 8 - 1;
        r2.x1 = r2.x0 + 9;
        r2.y1 = r2.y0 + 9;
        draw_rect(&r2, 7);
        rect_shrink(&r2, 1);
        draw_rect(&r2, 0);
    }

    // Sprite overview
    {
        x = 200 - 128 - 3;
        y = 8;
        draw_rect(&(rect_t){x, y, x + 1 + 128, y + 1 + 128}, 0);

        rect_t r = {x + 1, y + 1, x + 128, y + 128};
        if (mouse_hover(&r, MOUSE_SPR_HAND)) {
            unsigned hover_idx =
                ((edit_state.mouse_ev.y - r.y0) / 8) * 16 +
                ((edit_state.mouse_ev.x - r.x0) / 8);

            snprintf(edit_state.status_text, sizeof(edit_state.status_text), "Sprite %u", hover_idx);

            if (edit_state.mouse_ev.buttons == 1)
                edit_state.spr_edit.spr_idx = hover_idx;
        }

        fill_rect(&r, 0);
        x += 1;
        y += 1;

        for (int i = 0; i < 256; i++) {
            int row = i / 16;
            int col = i % 16;

            draw_game_sprite(i, x + col * 8, y + row * 8);
        }

        {
            r.x0 += (edit_state.spr_edit.spr_idx % 16) * 8;
            r.y0 += (edit_state.spr_edit.spr_idx / 16) * 8;

            r.x1 = r.x0 + 8;
            r.y1 = r.y0 + 8;
            r.x0 -= 1;
            r.y0 -= 1;
            draw_rect(&r, 7);
        }
    }

    // Sprite editor
    {
        x = 1;
        y = 27;

        rect_t r = {x, y, x + 1 + 64, y + 1 + 64};
        draw_rect(&r, 0);
        rect_shrink(&r, 1);

        const uint32_t *spr = &data_state.sprites[(edit_state.spr_edit.spr_idx >> 4) * 128 + (edit_state.spr_edit.spr_idx & 15)];

        for (int j = 0; j < 8; j++) {
            for (int i = 0; i < 8; i++) {
                rect_t r2;
                r2.x0 = r.x0 + i * 8;
                r2.y0 = r.y0 + j * 8;
                r2.x1 = r2.x0 + 7;
                r2.y1 = r2.y0 + 7;

                unsigned col = (*spr >> (i * 4)) & 0xF;

                if (mouse_hover(&r2, MOUSE_SPR_CROSSHAIR)) {
                    col = edit_state.spr_edit.color;
                }

                fill_rect(&r2, col);
            }
            spr += 16;
        }
    }

    // Commands
    {
        unsigned color       = 13;
        unsigned color_sel   = 7;
        unsigned color_hover = 12;

        x = 5;
        y = 98;

        unsigned tool = 0;
        for (int j = 0; j < 2; j++) {
            for (int i = 0; i < 6; i++) {
                unsigned col = color;

                rect_t r;
                r.x0 = x + i * 10;
                r.y0 = y;
                r.x1 = r.x0 + 7;
                r.y1 = r.y0 + 7;

                // fill_rect(&r, 0);

                if (mouse_hover(&r, MOUSE_SPR_HAND)) {
                    strcpy(edit_state.status_text, tool_status_text[tool]);

                    col = color_hover;
                    if (edit_state.mouse_ev.clicked_buttons == 1 && edit_state.mouse_ev.buttons == edit_state.mouse_ev.clicked_buttons)
                        on_tool_click(tool);
                }
                if (tool == edit_state.spr_edit.tool)
                    col = color_sel;

                draw_icon(x + i * 10, y, (j + 1) * 16 + i, col);
                tool++;
            }
            y += 9;
        }
    }
}

screen_t scr_sprite = {
    .draw = draw,
};
