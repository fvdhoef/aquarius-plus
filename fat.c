#include "fat.h"
#include "direnum.h"

struct fat_dirent {
    uint8_t  name[11];
    uint8_t  attr;
    uint8_t  nt_res;
    uint8_t  crt_time_tenth;
    uint16_t crt_time;
    uint16_t crt_date;
    uint16_t lst_acc_date;
    uint16_t fst_clus_hi;
    uint16_t wrt_time;
    uint16_t wrt_date;
    uint16_t fst_clus_lo;
    uint32_t filesize;
};

struct entry {
    char              name[256]; // Original name
    struct fat_dirent de;        // Emulated FAT entry
};

static char *basepath;
static char  subpath[1024];

static struct entry *entries;
static int           entries_capacity;
static int           entries_count;

static struct entry *add_entry(void) {
    if (entries_count >= entries_capacity) {
        // Reallocat buffer with twice the capacity
        if (entries_capacity == 0) {
            entries_capacity = 32; // Initial capacity
        } else {
            entries_capacity *= 2;
        }

        entries = realloc(entries, entries_capacity * sizeof(struct entry));
        if (entries == NULL) {
            perror("fat add_entry");
            exit(1);
        }
    }

    struct entry *result = &entries[entries_count++];
    memset(result, 0, sizeof(*result));
    return result;
}

static void free_entries(void) {
    if (entries != NULL) {
        free(entries);
        entries = NULL;
    }
    entries_capacity = 0;
    entries_count    = 0;
}

static char convert_ch(char ch) {
    static const char *allowed_chars = "$%'-_@~`!(){}^#&";

    // Convert to uppercase
    if (ch >= 'a' && ch <= 'z')
        ch += ('A' - 'a');

    if (!((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'Z') ||
          strchr(allowed_chars, ch) != NULL)) {

        // Non-allowed character, convert to underscore
        ch = '_';
    }
    return ch;
}

static void process_path(const char *path) {
    free_entries();

    direnum_ctx_t dectx = direnum_open(basepath);
    if (dectx != NULL) {
        struct direnum_ent de;
        while (direnum_read(dectx, &de)) {

            struct entry *entry = add_entry();
            snprintf(entry->name, sizeof(entry->name), "%s", de.filename);

            // Convert to 8.3 name
            const char *p = de.filename;
            char        ch;
            bool        strip_leading_periods = true;

            char shortname[12];
            int  idx = 0;

            // Get first 8 characters
            while ((ch = *(p++)) != 0 && idx < 8) {
                // Strip all leading and embedded spaces from the long name.
                if (ch == ' ')
                    continue;

                // Strip all leading periods from the long name.
                if (ch == '.') {
                    if (strip_leading_periods) {
                        continue;
                    } else {
                        break;
                    }
                    continue;
                }
                strip_leading_periods = false;

                shortname[idx++] = convert_ch(ch);
            }

            // Pad with spaces
            while (idx < 8) {
                shortname[idx++] = ' ';
            }

            // Skip up to the extension
            if (ch != 0) {
                if (ch != '.') {
                    while ((ch = *(p++)) != 0) {
                        if (ch == '.')
                            break;
                    }
                }

                // Get 3 extension characters
                while ((ch = *(p++)) != 0 && idx < 11) {
                    if (ch == '.')
                        break;

                    shortname[idx++] = convert_ch(ch);
                }
            }

            // Pad with spaces
            while (idx < 11) {
                shortname[idx++] = ' ';
            }
            shortname[idx] = 0;

            // Check if short name already exists
            int  trailing_number = 0;
            bool ok;
            do {
                ok = true;
                for (int i = 0; i < entries_count; i++) {
                    if (memcmp(entries[i].de.name, shortname, 11) == 0) {
                        // Name already exists, modify shortname and try again
                        ok = false;
                        trailing_number++;
                        if (trailing_number < 9) {
                            shortname[6] = '~';
                            shortname[7] = '0' + trailing_number;
                        } else if (trailing_number < 99) {
                            shortname[5] = '~';
                            shortname[6] = '0' + (trailing_number / 10);
                            shortname[7] = '0' + (trailing_number % 10);
                        } else {
                            fprintf(stderr, "Too many colliding short names, giving up.\n");
                            exit(1);
                        }
                        break;
                    }
                }
            } while (!ok);

            // Fill in FAT entry
            memcpy(entry->de.name, shortname, sizeof(entry->de.name));
            entry->de.attr = (de.attr & DE_DIR) ? 0x10 : 0;
            struct tm tm;
            localtime_r(&de.t, &tm);
            entry->de.wrt_time = (tm.tm_hour << 11) | (tm.tm_min << 5) | (tm.tm_sec / 2);
            entry->de.wrt_date = ((tm.tm_year + 1900 - 1980) << 9) | ((tm.tm_mon + 1) << 5) | tm.tm_mday;
            entry->de.filesize = de.size;

            printf("%s\n", shortname);
            printf("%s %u %u %lu\n", de.filename, de.size, de.attr, de.t);
        }
        direnum_close(dectx);
    }
}

int fat_init(const char *_basepath) {
    basepath = strdup(_basepath);
    process_path(basepath);

    exit(1);

    return 0;
}

int fat_open(const char *name) {
    return 0;
}

int fat_close(void) {
    return 0;
}

int fat_read(void *buf, size_t size) {
    return 0;
}

int fat_delete(const char *name) {
    return 0;
}
