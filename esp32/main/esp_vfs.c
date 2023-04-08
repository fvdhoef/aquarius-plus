#include "esp_vfs.h"

static int idx = 0;

static int esp_opendir(const char *path) {
    printf("esp_opendir: %s\n", path);
    idx = 0;
    return 0;
}

static int esp_closedir(int dd) {
    return 0;
}

static int esp_readdir(int dd, struct direnum_ent *de) {
    if (idx++ == 0) {
        snprintf(de->filename, sizeof(de->filename), "terminal");
        de->attr = 0;
        de->size = 0;
        de->t    = 0;
        return 0;
    }

    return ERR_EOF;
}

static int esp_stat(const char *path, struct stat *st) {
    if (strcasecmp(path, "esp:") == 0) {
        memset(st, 0, sizeof(*st));
        st->st_mode = S_IFDIR;
        return 0;
    }
    return ERR_OTHER;
}

struct vfs esp_vfs = {
    .opendir  = esp_opendir,
    .closedir = esp_closedir,
    .readdir  = esp_readdir,
    .stat     = esp_stat,
};
