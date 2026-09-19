#include "editbuf.h"
#include "ctype2.h"

static inline bool inc_ptr(struct editbuf *eb, uint8_t **p) {
    uint8_t *p_old = *p;

    (*p)++;
    if (*p >= eb->p_split_start && *p < eb->p_split_end)
        *p = eb->p_split_end;
    if (*p >= eb->p_buf_end)
        *p = eb->p_buf_end;

    return *p != p_old;
}

static inline bool dec_ptr(struct editbuf *eb, uint8_t **p) {
    uint8_t *p_old = *p;

    (*p)--;
    if (*p >= eb->p_split_start && *p < eb->p_split_end)
        *p = eb->p_split_start - 1;
    if (*p <= eb->p_buf)
        *p = eb->p_buf;

    return *p != p_old;
}

static inline int _abs(int x) { return x < 0 ? -x : x; }

static uint8_t *get_line_addr(struct editbuf *eb, int line) {
    line = clamp(line, 0, eb->line_count);

    // Determine best start point to search from
    {
        int dist_to_start = line;
        int dist_to_end   = eb->line_count - line;
        int dist_to_split = _abs(eb->cached_p_line - line);

        if (line == 0 || dist_to_start < dist_to_split) {
            eb->cached_p      = eb->p_buf;
            eb->cached_p_line = 0;
        }
        if (dist_to_end < dist_to_split) {
            uint8_t *p = eb->p_buf_end;
            if (dec_ptr(eb, &p) && *p == '\n') {
                eb->cached_p      = eb->p_buf_end;
                eb->cached_p_line = eb->line_count - 1;
            } else {
                p = eb->p_buf_end;

                while (1) {
                    if (!dec_ptr(eb, &p))
                        break;
                    if (*p == '\n') {
                        inc_ptr(eb, &p);
                        break;
                    }
                }

                eb->cached_p      = p;
                eb->cached_p_line = eb->line_count - 1;
            }
        }
    }

    uint8_t *p     = eb->cached_p;
    int      count = line - eb->cached_p_line;

    if (count > 0) {
        while (count-- > 0) {
            while (*p != '\n')
                if (!inc_ptr(eb, &p))
                    break;
            inc_ptr(eb, &p);
        }

    } else if (count < 0) {
        count = -count;
        while (count-- > 0) {
            // Skip past '\n' of previous line
            if (!dec_ptr(eb, &p))
                break;
            while (1) {
                // Check if previous char is a '\n'
                if (!dec_ptr(eb, &p))
                    break;
                if (*p == '\n') {
                    inc_ptr(eb, &p);
                    break;
                }
            }
        }
    }

    eb->cached_p      = p;
    eb->cached_p_line = line;
    return p;
}

static void invalidate_cached(struct editbuf *eb) {
    eb->cached_p      = eb->p_buf;
    eb->cached_p_line = 0;
}

void editbuf_init(struct editbuf *eb, uint8_t *p, size_t size) {
    eb->p_buf     = p;
    eb->p_buf_end = p + size;
    editbuf_reset(eb);
}

void editbuf_reset(struct editbuf *eb) {
    eb->line_count = 1;
    invalidate_cached(eb);
    eb->modified      = false;
    eb->p_split_start = eb->p_buf;
    eb->p_split_end   = eb->p_buf_end;
}

int editbuf_get_line_count(struct editbuf *eb) {
    return eb->line_count;
}

bool editbuf_get_modified(struct editbuf *eb) {
    return eb->modified;
}

static int get_line_len(struct editbuf *eb, const uint8_t *p_line) {
    const uint8_t *p = p_line;
    while (*p != '\n' && p != eb->p_buf_end && p != eb->p_split_start)
        p++;
    return p - p_line;
}

static uint8_t *get_line_end(struct editbuf *eb, uint8_t *p_line) {
    uint8_t *p = p_line;
    while (p != eb->p_buf_end && p != eb->p_split_start) {
        uint8_t ch = *(p++);
        if (ch == '\n')
            break;
    }
    return p;
}

static uint8_t *move_split_after_line(struct editbuf *eb, int line) {
    line = clamp(line, 0, eb->line_count - 1);

    uint8_t *p_line     = get_line_addr(eb, line);
    uint8_t *p_line_end = get_line_end(eb, p_line);
    if (p_line_end == eb->p_split_start) {
        // Already splitted at the right point
        return p_line;
    }

    if (p_line <= eb->p_split_start) {
        // Line is currently before split
        int copy_amount = eb->p_split_start - p_line_end;
        eb->p_split_start -= copy_amount;
        eb->p_split_end -= copy_amount;
        memmove(eb->p_split_end, eb->p_split_start, copy_amount);

        // Cached location stays valid

    } else {
        // Line is currently after split
        int copy_amount = p_line_end - eb->p_split_end;
        memmove(eb->p_split_start, eb->p_split_end, copy_amount);
        eb->p_split_start += copy_amount;
        eb->p_split_end += copy_amount;

        // Cached location has changed
        eb->cached_p_line = line;
        eb->cached_p      = eb->p_split_start - (p_line_end - p_line);
    }
    return eb->cached_p;
}

int editbuf_get_line(struct editbuf *eb, int line, const uint8_t **p) {
    if (line < 0 || line >= eb->line_count)
        return -1;

    uint8_t *p_line = get_line_addr(eb, line);
    *p              = p_line;

    return get_line_len(eb, p_line);
}

bool editbuf_insert_ch(struct editbuf *eb, location_t loc, char ch) {
    if (loc.line < 0 || loc.line >= eb->line_count || loc.pos < 0 || eb->p_split_start == eb->p_split_end)
        return false;

    uint8_t *p_line   = move_split_after_line(eb, loc.line);
    int      line_len = get_line_len(eb, p_line);
    if (loc.pos >= line_len)
        loc.pos = line_len;

    uint8_t *p_loc = p_line + loc.pos;

    memmove(p_loc + 1, p_loc, eb->p_split_start - p_loc);
    eb->p_split_start++;
    *p_loc = ch;

    if (ch == '\n')
        eb->line_count++;

    eb->modified = true;
    return true;
}

bool editbuf_delete_ch(struct editbuf *eb, location_t loc) {
    if (loc.line < 0 || loc.line >= eb->line_count || loc.pos < 0)
        return false;

    uint8_t *p_line     = move_split_after_line(eb, loc.line);
    uint8_t *p_line_end = get_line_end(eb, p_line);
    uint8_t *p_loc      = p_line + loc.pos;
    if (p_loc >= p_line_end)
        return false;

    if (*p_loc == '\n') {
        // Merge with next line
        if (p_line == eb->p_buf_end)
            return false;
        if (p_line == eb->p_split_start && eb->p_split_end == eb->p_buf_end)
            return false;

        // Remove '\n'
        eb->p_split_start--;
        eb->line_count--;

        // Copy data from next line
        uint8_t *p_next_end = get_line_end(eb, eb->p_split_end);

        int copy_amount = p_next_end - eb->p_split_end;
        memmove(eb->p_split_start, eb->p_split_end, copy_amount);
        eb->p_split_start += copy_amount;
        eb->p_split_end += copy_amount;

    } else if (p_loc < p_line_end) {
        // Delete within line
        uint8_t *p_from      = p_loc + 1;
        int      copy_amount = p_line_end - p_from;
        memmove(p_loc, p_from, copy_amount);
        eb->p_split_start = p_loc + copy_amount;
    }

    eb->modified = true;
    return true;
}

bool editbuf_delete_range(struct editbuf *eb, location_t loc_from, location_t loc_to) {
    if (loc_from.line < 0 || loc_from.line >= eb->line_count || loc_from.pos < 0 ||
        loc_to.line < 0 || loc_to.line >= eb->line_count || loc_to.pos < 0)
        return false;

    // Swap loc_to/loc_from if loc_from is larger than loc_to
    if (loc_lt(loc_to, loc_from)) {
        location_t tmp = loc_from;
        loc_from       = loc_to;
        loc_to         = tmp;
    }

    if (loc_from.line == loc_to.line) {
        // Within line
        uint8_t *p_line     = move_split_after_line(eb, loc_from.line);
        uint8_t *p_line_end = get_line_end(eb, p_line);
        int      p_line_len = get_line_len(eb, p_line);
        uint8_t *p_loc_from = p_line + loc_from.pos;
        if (p_loc_from >= p_line_end)
            return false;

        uint8_t *p_loc_to = p_line + loc_to.pos;
        if (p_loc_to > p_line + p_line_len) {
            return false;
        }

        int copy_amount = p_line_end - p_loc_to;
        memmove(p_loc_from, p_loc_to, copy_amount);
        eb->p_split_start = p_loc_from + copy_amount;

    } else {
        // Across lines
        uint8_t *p_line     = move_split_after_line(eb, loc_from.line);
        uint8_t *p_line_end = get_line_end(eb, p_line);
        uint8_t *p_loc_from = p_line + loc_from.pos;
        if (p_loc_from >= p_line_end)
            return false;

        uint8_t *p_line_to     = get_line_addr(eb, loc_to.line);
        uint8_t *p_line_to_end = get_line_end(eb, p_line_to);
        uint8_t *p_loc_to      = p_line_to + loc_to.pos;
        if (p_loc_to >= p_line_to_end && p_loc_to != eb->p_buf_end)
            return false;

        eb->line_count--;
        uint8_t *p = eb->p_split_end;
        while (p < p_loc_to) {
            uint8_t val = *(p++);
            if (val == '\n')
                eb->line_count--;
        }

        int copy_amount = p_line_to_end - p_loc_to;
        memmove(p_loc_from, p_loc_to, copy_amount);
        eb->p_split_start = p_loc_from + copy_amount;
        eb->p_split_end   = p_line_to_end;
    }

    eb->modified = true;
    return true;
}

// Remove control characters, expand tabs to spaces, normalize line endings
static bool normalize(struct editbuf *eb, const uint8_t *ps, const uint8_t *ps_end) {
    editbuf_reset(eb);

    uint8_t *pd     = eb->p_buf;
    uint8_t *p_line = pd;

    while (ps < ps_end) {
        uint8_t val = *(ps++);
        if (is_cntrl(val) && val != '\n' && val != '\t')
            continue;
        if (pd >= eb->p_buf_end || pd >= ps)
            return false;

        if (val == '\t') {
            do {
                *(pd++) = ' ';
                if (pd >= eb->p_buf_end || pd >= ps)
                    return false;
            } while ((pd - p_line) % 2 != 0);
            continue;
        }

        *(pd++) = val;

        if (val == '\n') {
            eb->line_count++;
            p_line = pd;
        }
    }
    if (p_line != pd)
        eb->line_count++;

    eb->p_split_start = pd;

    return true;
}

static int normalize2(uint8_t **pdst, const uint8_t *ps, const uint8_t *ps_end, location_t *loc) {
    uint8_t *pd = *pdst;

    int line_count = 0;
    while (ps < ps_end) {
        uint8_t val = *(ps++);
        if (is_cntrl(val) && val != '\n' && val != '\t')
            continue;

        if (val == '\t')
            val = ' ';

        *(pd++) = val;
        loc->pos++;
        if (val == '\n') {
            loc->line++;
            loc->pos = 0;
            line_count++;
        }
    }

    *pdst = pd;
    return line_count;
}

bool editbuf_load(struct editbuf *eb, const char *path) {
    FILE *f = fopen(path, "rt");
    if (f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    int p_buf_size = eb->p_buf_end - eb->p_buf;
    memset(eb->p_buf, 'A', p_buf_size);

    if (file_size > p_buf_size)
        goto error;

    uint8_t *p_load = eb->p_buf_end - file_size;
    fread(p_load, file_size, 1, f);
    fclose(f);
    f = NULL;

    if (!normalize(eb, p_load, eb->p_buf_end))
        goto error;

    return true;

error:
    editbuf_reset(eb);
    return false;
}

bool editbuf_save(struct editbuf *eb, const char *path) {
    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return false;

    unsigned size = (eb->p_split_start - eb->p_buf);
    if (size)
        fwrite(eb->p_buf, size, 1, f);
    size = (eb->p_buf_end - eb->p_split_end);
    if (size)
        fwrite(eb->p_split_end, size, 1, f);

    fclose(f);

    eb->modified = false;
    return true;
}

bool editbuf_save_range(struct editbuf *eb, location_t from, location_t to, const char *path) {
    uint8_t *p_line_to   = move_split_after_line(eb, to.line);
    uint8_t *p_line_from = get_line_addr(eb, from.line);
    uint8_t *p_loc_from  = p_line_from + from.pos;
    uint8_t *p_loc_to    = p_line_to + to.pos;

    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return false;
    fwrite(p_loc_from, p_loc_to - p_loc_from, 1, f);
    fclose(f);

    return true;
}

bool editbuf_insert_from_file(struct editbuf *eb, location_t *loc, const char *path) {
    FILE *f = fopen(path, "rt");
    if (f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    int file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    uint8_t *p_line     = move_split_after_line(eb, loc->line);
    uint8_t *p_line_end = get_line_end(eb, p_line);

    if (file_size >= eb->p_split_end - eb->p_split_start) {
        // Too big to insert
        fclose(f);
        return false;
    }

    uint8_t *p_loc       = p_line + loc->pos;
    int      copy_amount = p_line_end - p_loc;

    uint8_t *p_load = eb->p_split_end - copy_amount;
    memmove(p_load, p_loc, copy_amount);
    p_load -= file_size;
    fread(p_load, file_size, 1, f);
    fclose(f);

    uint8_t *pd = p_loc;
    eb->line_count += normalize2(&pd, p_load, p_load + file_size, loc);

    p_load += file_size;
    memmove(pd, p_load, copy_amount);
    pd += copy_amount;
    eb->p_split_start = pd;

    eb->modified = true;

    return true;
}
