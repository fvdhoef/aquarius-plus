#include "fat.h"
#include "direnum.h"

struct entry {
    char             *name; // Original name
    struct fat_dirent de;   // Emulated FAT entry
};
static struct entry *entries;
static int           entries_capacity;
static int           entries_count;

static char *basepath;
static char *current_path;
static FILE *opened_file;
static char *opened_file_path;

static int enum_entry = -1;

static struct entry *add_entry(void) {
    if (entries_count >= entries_capacity) {
        // Reallocat buffer with twice the capacity
        if (entries_capacity == 0) {
            entries_capacity = 32; // Initial capacity
        } else {
            entries_capacity *= 2;
        }

        struct entry *new_entries = realloc(entries, entries_capacity * sizeof(struct entry));
        assert(new_entries != NULL);
        entries = new_entries;
    }

    struct entry *result = &entries[entries_count++];
    memset(result, 0, sizeof(*result));
    return result;
}

static void free_entries(void) {
    if (entries != NULL) {
        for (int i = 0; i < entries_count; i++) {
            if (entries[i].name != NULL) {
                free(entries[i].name);
                entries[i].name = NULL;
            }
        }

        free(entries);
        entries = NULL;
    }
    entries_capacity = 0;
    entries_count    = 0;
}

static inline char to_upper(char ch) {
    if (ch >= 'a' && ch <= 'z')
        ch += ('A' - 'a');
    return ch;
}

static bool is_allowed_char(char ch) {
    static const char *allowed_chars = "$%'-_@~`!(){}^#&";
    return (
        (ch >= '0' && ch <= '9') ||
        (ch >= 'A' && ch <= 'Z') ||
        strchr(allowed_chars, ch) != NULL);
}

static char convert_ch(char ch) {
    // Convert to uppercase
    ch = to_upper(ch);
    if (!is_allowed_char(ch)) {
        // Non-allowed character, convert to underscore
        ch = '_';
    }
    return ch;
}

static int entry_compare(const void *_entry1, const void *_entry2) {
    const struct entry *entry1 = _entry1;
    const struct entry *entry2 = _entry2;
    return memcmp(entry1->de.name, entry2->de.name, sizeof(entry1->de.name));
}

static void process_path(const char *path) {
    if (current_path == NULL || strcmp(current_path, path) != 0) {
        if (current_path != NULL) {
            free(current_path);
            current_path = NULL;
        }
        current_path = strdup(path);
    }
    bool is_basepath = strcmp(path, basepath) == 0;

    free_entries();

    direnum_ctx_t dectx = direnum_open(current_path);
    if (dectx != NULL) {
        struct direnum_ent de;
        while (direnum_read(dectx, &de)) {

            struct entry *entry      = add_entry();
            int           str_length = snprintf(NULL, 0, "%s/%s", current_path, de.filename) + 1;
            entry->name              = malloc(str_length);
            snprintf(entry->name, str_length, "%s/%s", current_path, de.filename);

            // Convert to 8.3 name
            const char *p = de.filename;
            char        ch;
            bool        strip_leading_periods = true;

            char shortname[12];
            int  idx = 0;

            if (strcmp(p, ".") == 0) {
                if (!is_basepath) {
                    shortname[idx++] = '.';
                } else {
                    // Don't include this entry in root directory listing.
                    entries_count--;
                    continue;
                }

            } else if (strcmp(p, "..") == 0) {
                if (!is_basepath) {
                    shortname[idx++] = '.';
                    shortname[idx++] = '.';
                } else {
                    // Don't include this entry in root directory listing.
                    entries_count--;
                    continue;
                }

            } else {
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
            entry->de.attr     = (de.attr & DE_DIR) ? 0x10 : 0;
            struct tm *tm      = localtime(&de.t);
            entry->de.wrt_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
            entry->de.wrt_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;
            entry->de.filesize = de.size;

            // printf("%s %s\n", shortname, entry->name);
        }
        direnum_close(dectx);
    }

    // Sort entries
    qsort(entries, entries_count, sizeof(entries[0]), entry_compare);
}

int fat_init(const char *_basepath) {
    basepath = strdup(_basepath);
    process_path(basepath);

    return 0;
}

static int find_file(const char *name, struct entry **found_entry) {
    *found_entry = NULL;

    // If name start with '/' start at
    const char *ps = name;
    if (*ps == '/') {
        process_path(basepath);
        ps++;
    } else {
        process_path(current_path);
    }

    if (ps[0] == 0) {
        return OPEN_IS_DIR;
    } else if (ps[0] == '*') {
        // Open current directory
        enum_entry = 0;
        return OPEN_ENUM_DIR;
    } else {
        // Open given file/directory

        // Convert given name to FAT directory entry name
        char shortname[12];

        if (strlen(ps) == 11 && strchr(ps, '.') == NULL) {
            // HACK: Use name as is.
            memcpy(shortname, ps, 11);
            shortname[11] = 0;

        } else {
            // Get first 8 characters
            int  idx = 0;
            char ch;
            while ((ch = *(ps++)) != 0 && idx < 8) {
                if (ch == '.')
                    break;
                ch = to_upper(ch);
                if (!is_allowed_char(ch))
                    return ERR_INVALID_NAME;
                shortname[idx++] = ch;
            }
            // Pad with spaces
            while (idx < 8) {
                shortname[idx++] = ' ';
            }
            if (ch != 0 && ch != '.') {
                return ERR_INVALID_NAME;
            }
            // Get 3 extension characters
            while ((ch = *(ps++)) != 0 && idx < 11) {
                ch = to_upper(ch);
                if (!is_allowed_char(ch))
                    return ERR_INVALID_NAME;
                shortname[idx++] = ch;
            }
            if (ch != 0) {
                return ERR_INVALID_NAME;
            }
            // Pad with spaces
            while (idx < 11) {
                shortname[idx++] = ' ';
            }
            shortname[idx++] = 0;
        }

        // Find name in current directory
        struct entry *entry = NULL;
        for (int i = 0; i < entries_count; i++) {
            if (memcmp(shortname, entries[i].de.name, 11) == 0) {
                entry = &entries[i];
                break;
            }
        }
        if (entry == NULL) {
            return ERR_INVALID_NAME;
        }

        printf("Shortname: %s -> %s  (attr: %02X)\n", shortname, entry->name, entry->de.attr);
        *found_entry = entry;
    }
    return 0;
}

int fat_open(const char *name) {
    fat_close();

    struct entry *entry;
    int           result = find_file(name, &entry);
    if (result != 0) {
        return result;
    }

    if (entry->de.attr & 0x10) {
        // Open subdirectory
        process_path(entry->name);
        enum_entry = 0;
        return OPEN_IS_DIR;
    } else {
        opened_file_path = strdup(entry->name);
        opened_file      = fopen(opened_file_path, "rb");
        return OPEN_IS_FILE;
    }
    return 0;
}

int fat_create(const char *name) {
    fat_close();

    // If name start with '/' start at
    const char *ps = name;
    if (*ps == '/') {
        process_path(basepath);
        ps++;
    } else {
        process_path(current_path);
    }

    // Compose path
    int str_length   = snprintf(NULL, 0, "%s/%s", current_path, name) + 1;
    opened_file_path = malloc(str_length);
    assert(opened_file_path != NULL);
    snprintf(opened_file_path, str_length, "%s/%s", current_path, name);

    opened_file = fopen(opened_file_path, "wb");
    printf("Create file: %s\n", opened_file_path);
    if (opened_file == NULL) {
        return ERR_INVALID_NAME;
    }
    return 0;
}

int fat_truncate(void) {
    if (opened_file_path == NULL) {
        return ERR_INVALID_NAME;
    }

    opened_file = fopen(opened_file_path, "wb");
    printf("Create file: %s\n", opened_file_path);
    if (opened_file == NULL) {
        return ERR_INVALID_NAME;
    }
    return 0;
}

int fat_close(void) {
    if (opened_file) {
        fclose(opened_file);
        opened_file = NULL;

        free(opened_file_path);
        opened_file_path = NULL;
    }
    enum_entry = -1;
    return 0;
}

int fat_read(void *buf, size_t size) {
    if (opened_file != NULL) {
        return (int)fread(buf, 1, size, opened_file);

    } else if (enum_entry >= 0) {
        if (enum_entry == entries_count) {
            return 0;
        }

        if (size > sizeof(struct fat_dirent))
            size = sizeof(struct fat_dirent);

        memcpy(buf, &entries[enum_entry].de, size);
        enum_entry++;

        return (int)size;
    }
    return 0;
}

int fat_write(const void *buf, size_t size) {
    if (fwrite(buf, 1, size, opened_file) != size) {
        return ERR_DISKFULL;
    }
    return 0;
}

int fat_seek(uint32_t offset) {
    if (opened_file != NULL) {
        fseek(opened_file, offset, SEEK_SET);
    }
    return 0;
}

int fat_delete(const char *name) {
    fat_close();

    struct entry *entry;
    int           result = find_file(name, &entry);
    if (result != 0) {
        return result;
    }

    if (entry->de.attr & 0x10) {
        // Open subdirectory
        if (rmdir(entry->name) != 0) {
            return ERR_INVALID_NAME;
        }
    } else {
        if (unlink(entry->name) != 0) {
            return ERR_INVALID_NAME;
        }
    }

    return 0;
}
