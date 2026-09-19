#pragma once

#include "common.h"

#define MAX_BUFSZ  255
#define MAX_LINESZ (MAX_BUFSZ - 1)

typedef struct {
    int line, pos;
} location_t;

static inline bool loc_lt(location_t l, location_t r) {
    return (l.line == r.line) ? (l.pos < r.pos) : (l.line < r.line);
}

struct editbuf {
    uint8_t *p_buf;
    uint8_t *p_buf_end;
    int      line_count;
    uint8_t *cached_p;
    int      cached_p_line;
    bool     modified;
    uint8_t *p_split_start;
    uint8_t *p_split_end;
};

void editbuf_init(struct editbuf *eb, uint8_t *p, size_t size);
void editbuf_reset(struct editbuf *eb);
bool editbuf_get_modified(struct editbuf *eb);
int  editbuf_get_line_count(struct editbuf *eb);
int  editbuf_get_line(struct editbuf *eb, int line, const uint8_t **p);
bool editbuf_insert_ch(struct editbuf *eb, location_t loc, char ch);
bool editbuf_delete_ch(struct editbuf *eb, location_t loc);
bool editbuf_delete_range(struct editbuf *eb, location_t from, location_t to);
bool editbuf_load(struct editbuf *eb, const char *path);
bool editbuf_save(struct editbuf *eb, const char *path);
bool editbuf_save_range(struct editbuf *eb, location_t from, location_t to, const char *path);
bool editbuf_insert_from_file(struct editbuf *eb, location_t *loc, const char *path);
