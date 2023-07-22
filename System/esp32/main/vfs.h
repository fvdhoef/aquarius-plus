#pragma once

#include "common.h"
#include "direnum.h"
#include <sys/stat.h>

enum {
    ERR_NOT_FOUND     = -1, // File / directory not found
    ERR_TOO_MANY_OPEN = -2, // Too many open files / directories
    ERR_PARAM         = -3, // Invalid parameter
    ERR_EOF           = -4, // End of file / directory
    ERR_EXISTS        = -5, // File already exists
    ERR_OTHER         = -6, // Other error
    ERR_NO_DISK       = -7, // No disk
    ERR_NOT_EMPTY     = -8, // Not empty
};

enum {
    FO_RDONLY  = 0x00, // Open for reading only
    FO_WRONLY  = 0x01, // Open for writing only
    FO_RDWR    = 0x02, // Open for reading and writing
    FO_ACCMODE = 0x03, // Mask for above modes

    FO_APPEND = 0x04,  // Append mode
    FO_CREATE = 0x08,  // Create if non-existant
    FO_TRUNC  = 0x10,  // Truncate to zero length
    FO_EXCL   = 0x20,  // Error if already exists
};

struct vfs {
    // File operations
    int (*open)(uint8_t flags, const char *path);
    int (*close)(int fd);
    int (*read)(int fd, uint16_t size, void *buf);
    int (*write)(int fd, uint16_t size, const void *buf);
    int (*seek)(int fd, uint32_t offset);
    int (*tell)(int fd);

    // Directory operations
    int (*opendir)(const char *path);
    int (*closedir)(int dd);
    struct direnum_ent *(*readdir)(int dd);

    // Filesystem operations
    int (*delete_)(const char *path);
    int (*rename)(const char *path_old, const char *path_new);
    int (*mkdir)(const char *path);
    int (*stat)(const char *path, struct stat *st);
};
