#pragma once

#include "common.h"
#include "esp.h"

// Edit field
struct edit_field_ctx {
    int    y;
    int    x;
    int    w;
    char  *buf;
    size_t buf_len;
    int    cursor_pos;
    int    first_pos;
};

void edit_field_init(struct edit_field_ctx *ctx, int y, int x, int w, char *buf, size_t buf_len);
void edit_field_reset(struct edit_field_ctx *ctx);
void edit_field_draw(struct edit_field_ctx *ctx, bool show_cursor);
int  edit_field_handle(struct edit_field_ctx *ctx);

// File list
struct file_list_ctx {
    int    y;
    int    x;
    int    w;
    int    rows;
    int    old_selection;
    int    selection;
    int    total;
    char  *fn_buf;
    size_t fn_bufsize;

    struct esp_stat st;
};

void file_list_init(struct file_list_ctx *ctx, int y, int x, int w, int rows, char *fn_buf, size_t fn_bufsize);
void file_list_reset(struct file_list_ctx *ctx);
void file_list_draw(struct file_list_ctx *ctx, bool show_selection);
int  file_list_handle(struct file_list_ctx *ctx);
