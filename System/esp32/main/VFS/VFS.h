#pragma once

#include "Common.h"

enum {
    ERR_NOT_FOUND       = -1, // File / directory not found
    ERR_TOO_MANY_OPEN   = -2, // Too many open files / directories
    ERR_PARAM           = -3, // Invalid parameter
    ERR_EOF             = -4, // End of file / directory
    ERR_EXISTS          = -5, // File already exists
    ERR_OTHER           = -6, // Other error
    ERR_NO_DISK         = -7, // No disk
    ERR_NOT_EMPTY       = -8, // Not empty
    ERR_WRITE_PROTECTED = -9, // Disk write protected
};

enum {
    FO_RDONLY  = 0x00, // Open for reading only
    FO_WRONLY  = 0x01, // Open for writing only
    FO_RDWR    = 0x02, // Open for reading and writing
    FO_ACCMODE = 0x03, // Mask for above modes

    FO_APPEND = 0x04, // Append mode
    FO_CREATE = 0x08, // Create if non-existant
    FO_TRUNC  = 0x10, // Truncate to zero length
    FO_EXCL   = 0x20, // Error if already exists
};

enum {
    DE_FLAG_ALWAYS_DIRS = 0x01, // Always return directories even if they don't match the wildcard
    DE_FLAG_HIDDEN      = 0x02, // Return hidden files (with system/hidden attribute or starting with a dot)
    DE_FLAG_DOTDOT      = 0x04, // Include a '..' entry if this is not the root directory
    DE_FLAG_MODE83      = 0x08, // Return entries in 8.3 mode
};

enum {
    DE_ATTR_DIR = (1 << 0),
};

struct DirEnumEntry {
    DirEnumEntry()
        : size(0), attr(0), fdate(0), ftime(0) {
    }
    DirEnumEntry(const std::string &_filename, uint32_t _size, uint8_t _attr, uint16_t _fdate, uint16_t _ftime)
        : filename(_filename), size(_size), attr(_attr), fdate(_fdate), ftime(_ftime) {
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

    virtual void init() {}

    // File operations
    virtual int open(uint8_t flags, const std::string &path) { return ERR_OTHER; }
    virtual int close(int fd) { return ERR_OTHER; }
    virtual int read(int fd, size_t size, void *buf) { return ERR_OTHER; }
    virtual int readline(int fd, size_t size, void *buf) { return ERR_OTHER; }
    virtual int write(int fd, size_t size, const void *buf) { return ERR_OTHER; }
    virtual int seek(int fd, size_t offset) { return ERR_OTHER; }
    virtual int tell(int fd) { return ERR_OTHER; }

    // Directory operations
    virtual std::pair<int, DirEnumCtx> direnum(const std::string &path, uint8_t flags) { return std::make_pair(ERR_OTHER, nullptr); }

    // Filesystem operations
    virtual int delete_(const std::string &path) { return ERR_OTHER; }
    virtual int rename(const std::string &path_old, const std::string &path_new) { return ERR_OTHER; }
    virtual int mkdir(const std::string &path) { return ERR_OTHER; }
    virtual int stat(const std::string &path, struct stat *st) { return ERR_OTHER; }
};

VFS *getSDCardVFS();
VFS *getEspVFS();
VFS *getHttpVFS();
VFS *getTcpVFS();
