#include "VFS.h"
#include "xz.h"

#ifndef EMULATOR
extern const uint8_t romfs_start[] asm("_binary_romfs_bin_start");
extern const uint8_t romfs_end[] asm("_binary_romfs_bin_end");
#else
#include "romfs_contents.h"
const uint8_t *romfs_end = romfs_start + sizeof(romfs_start);
#endif

#pragma pack(push, 1)
struct FileEntry {
    uint8_t  recSize;
    uint32_t offset;
    uint32_t fsize;
    uint16_t fdate;
    uint16_t ftime;
    uint32_t compressedSize;
    char     filename[128];
};
#pragma pack(pop)

struct OpenFile {
    const FileEntry     *fe     = nullptr;
    unsigned             offset = 0;
    std::vector<uint8_t> data;
};

static const FileEntry *findFile(const std::string &_path) {
    // Skip leading slashes
    auto idx = _path.find_first_not_of('/');
    if (idx == std::string::npos) {
        idx = _path.size();
    }
    auto path = _path.substr(idx);

    // Find file
    const uint8_t *p = romfs_start;
    while (1) {
        const FileEntry *fe = (const FileEntry *)p;
        if (fe->recSize == 0)
            break;
        p += fe->recSize;

        if (strcasecmp(path.c_str(), fe->filename) == 0)
            return fe;
    }
    return nullptr;
}

class EspVFS : public VFS {
public:
    OpenFile openFile;

    EspVFS() {
    }

    void init() override {
    }

    int open(uint8_t flags, const std::string &_path) override {
        (void)flags;

        // Skip leading slashes
        auto idx = _path.find_first_not_of('/');
        if (idx == std::string::npos) {
            idx = _path.size();
        }
        auto path = _path.substr(idx);

        auto fe = findFile(_path);
        if (!fe)
            return ERR_NOT_FOUND;

        if (openFile.fe)
            return ERR_TOO_MANY_OPEN;

        openFile.fe     = fe;
        openFile.offset = 0;
        openFile.data.resize(openFile.fe->fsize);

        ESP_LOGW("espvfs", "Decompressing '%s' %u -> %u", openFile.fe->filename, (unsigned)openFile.fe->compressedSize, (unsigned)openFile.fe->fsize);

        if (xz_decompress((uint8_t *)romfs_start + openFile.fe->offset, openFile.fe->compressedSize, (uint8_t *)openFile.data.data()) != XZ_SUCCESS) {
            openFile.fe = NULL;
            openFile.data.clear();
            return ERR_OTHER;
        }
        return 0;
    }

    int read(int fd, size_t size, void *buf) override {
        if (fd == 0) {
            int remaining = (int)(openFile.data.size() - openFile.offset);
            if ((int)size > remaining) {
                size = remaining;
            }
            memcpy(buf, openFile.data.data() + openFile.offset, size);
            openFile.offset += (int)size;
            return (int)size;
        } else {
            return ERR_PARAM;
        }
    }

    int write(int fd, size_t size, const void *buf) override {
        return ERR_WRITE_PROTECTED;
    }

    int seek(int fd, size_t offset) override {
        if (fd != 0 || !openFile.fe)
            return ERR_PARAM;

        if (offset > openFile.fe->fsize)
            offset = openFile.fe->fsize;

        openFile.offset = (unsigned)offset;
        return 0;
    }

    int tell(int fd) override {
        if (fd != 0 || !openFile.fe)
            return ERR_PARAM;
        return openFile.offset;
    }

    int close(int fd) override {
        if (fd == 0) {
            openFile.fe = NULL;
            openFile.data.clear();
        }
        return 0;
    }

    std::pair<int, DirEnumCtx> direnum(const std::string &path, uint8_t flags) override {
        (void)path;
        if (flags & DE_FLAG_MODE83)
            return std::make_pair(ERR_PARAM, nullptr);

        auto result = std::make_shared<std::vector<DirEnumEntry>>();

        const uint8_t *p = romfs_start;
        while (1) {
            struct FileEntry *fe = (struct FileEntry *)p;
            if (fe->recSize == 0)
                break;
            p += fe->recSize;

            result->emplace_back(fe->filename, (uint32_t)fe->fsize, 0, (uint16_t)fe->fdate, (uint16_t)fe->ftime);
        }
        return std::make_pair(0, result);
    }

    int stat(const std::string &_path, struct stat *st) override {
        // Skip leading slashes
        auto idx = _path.find_first_not_of('/');
        if (idx == std::string::npos) {
            idx = _path.size();
        }
        auto path = _path.substr(idx);

        if (strcasecmp(path.c_str(), "") == 0) {
            memset(st, 0, sizeof(*st));
            st->st_mode = S_IFDIR;
            return 0;

        } else {
            auto fe = findFile(_path);
            if (!fe) {
                return ERR_NOT_FOUND;
            }

            memset(st, 0, sizeof(*st));
            st->st_size = fe->fsize;
            st->st_mode = S_IFREG;

            struct tm tm;
            memset(&tm, 0, sizeof(tm));
            tm.tm_mday   = fe->fdate & 31,
            tm.tm_mon    = ((fe->fdate >> 5) & 15) - 1,
            tm.tm_year   = (fe->fdate >> 9) + 80,
            tm.tm_sec    = (fe->ftime & 31) * 2,
            tm.tm_min    = (fe->ftime >> 5) & 63,
            tm.tm_hour   = fe->ftime >> 11,
            tm.tm_isdst  = -1;
            st->st_mtime = mktime(&tm);
            return 0;
        }
    }
};

VFS *getEspVFS() {
    static EspVFS obj;
    return &obj;
}
