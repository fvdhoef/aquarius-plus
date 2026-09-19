#pragma once

#include "common.h"

typedef struct {
    int line, pos;
} location_t;

static inline bool loc_lt(location_t l, location_t r) {
    return (l.line == r.line) ? (l.pos < r.pos) : (l.line < r.line);
}

typedef struct {
    uint8_t *p_buf;
    uint8_t *p_buf_end;
    int      line_count;
    uint8_t *cached_p;
    int      cached_p_line;
    bool     modified;
    uint8_t *p_split_start;
    uint8_t *p_split_end;
} editbuf_t;

void     editbuf_init(editbuf_t *eb, uint8_t *p, size_t size);
void     editbuf_reset(editbuf_t *eb);
bool     editbuf_get_modified(editbuf_t *eb);
int      editbuf_get_line_count(editbuf_t *eb);
int      editbuf_get_line(editbuf_t *eb, int line, const uint8_t **p);
bool     editbuf_insert_ch(editbuf_t *eb, location_t loc, char ch);
bool     editbuf_delete_ch(editbuf_t *eb, location_t loc);
bool     editbuf_delete_range(editbuf_t *eb, location_t from, location_t to);
bool     editbuf_load(editbuf_t *eb, const char *path);
bool     editbuf_save(editbuf_t *eb, const char *path);
bool     editbuf_save_range(editbuf_t *eb, location_t from, location_t to, const char *path);
bool     editbuf_insert_from_file(editbuf_t *eb, location_t *loc, const char *path);
unsigned editbuf_get_size(editbuf_t *eb);
bool     editbuf_normalize(editbuf_t *eb, const uint8_t *ps, const uint8_t *ps_end);
unsigned editbuf_get_buf(editbuf_t *eb, const uint8_t **p);
