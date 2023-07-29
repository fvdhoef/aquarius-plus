// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#pragma once

#include "Common.h"

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

enum {
    DE_DIR = (1 << 0),
};

struct DirEnumEntry {
    DirEnumEntry()
        : size(0), attr(0), fdate(0), ftime(0) {
    }
    DirEnumEntry(const std::string &filename, uint32_t size, uint8_t attr, uint16_t fdate, uint16_t ftime)
        : filename(filename), size(size), attr(attr), fdate(fdate), ftime(ftime) {
    }

    std::string filename;
    uint32_t    size;
    uint8_t     attr;
    uint16_t    fdate;
    uint16_t    ftime;
};

using DirEnumCtx = std::shared_ptr<std::vector<DirEnumEntry>>;

class VFS {
public:
    VFS() {
    }
    virtual ~VFS() {
    }

    // File operations
    virtual int open(uint8_t flags, const std::string &path);
    virtual int close(int fd);
    virtual int read(int fd, size_t size, void *buf);
    virtual int write(int fd, size_t size, const void *buf);
    virtual int seek(int fd, size_t offset);
    virtual int tell(int fd);

    // Directory operations
    virtual DirEnumCtx direnum(const std::string &path);

    // Filesystem operations
    virtual int delete_(const std::string &path);
    virtual int rename(const std::string &path_old, const std::string &path_new);
    virtual int mkdir(const std::string &path);
    virtual int stat(const std::string &path, struct stat *st);
};
