#include "VFS.h"

#include "romfs_contents.h"
const uint8_t *romfs_end = romfs_start + sizeof(romfs_start);

#pragma pack(push, 1)
struct FileEntry {
    uint8_t  rec_size;
    uint32_t offset;
    uint32_t fsize;
    uint16_t fdate;
    uint16_t ftime;
    char     filename[128];
};
#pragma pack(pop)

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
        if (fe->rec_size == 0)
            break;
        p += fe->rec_size;

        if (strcasecmp(path.c_str(), fe->filename) == 0)
            return fe;
    }
    return nullptr;
}

struct OpenFile {
    const FileEntry *fe;
    unsigned         offset;
};

class EspVFS : public VFS {
public:
    OpenFile openFile;

    EspVFS() {
    }

    void init() {
    }

    int open(uint8_t flags, const std::string &_path) {
        (void)flags;

        // Skip leading slashes
        auto idx = _path.find_first_not_of('/');
        if (idx == std::string::npos) {
            idx = _path.size();
        }
        auto path = _path.substr(idx);

        // printf("esp_open(%u, \"%s\")\n", flags, path.c_str());

        auto fe = findFile(_path);
        if (!fe)
            return ERR_NOT_FOUND;

        if (openFile.fe)
            return ERR_TOO_MANY_OPEN;

        openFile.fe     = fe;
        openFile.offset = 0;
        return 0;
    }

    int read(int fd, size_t size, void *buf) {
        if (fd == 0) {
            int remaining = openFile.fe->fsize - openFile.offset;
            if ((int)size > remaining) {
                size = remaining;
            }
            memcpy(buf, romfs_start + openFile.fe->offset + openFile.offset, size);
            openFile.offset += (int)size;
            return (int)size;

        } else {
            return ERR_PARAM;
        }
    }

    int write(int fd, size_t size, const void *buf) {
        return ERR_WRITE_PROTECTED;
    }

    int seek(int fd, size_t offset) {
        if (fd != 0 || !openFile.fe)
            return ERR_PARAM;

        if (offset > openFile.fe->fsize)
            offset = openFile.fe->fsize;

        openFile.offset = (unsigned)offset;
        return 0;
    }

    int tell(int fd) {
        if (fd != 0 || !openFile.fe)
            return ERR_PARAM;
        return openFile.offset;
    }

    int close(int fd) {
        if (fd == 0) {
            openFile.fe = NULL;
        }
        return 0;
    }

    std::pair<int, DirEnumCtx> direnum(const std::string &path, uint8_t flags) {
        (void)path;
        if (flags & DE_FLAG_MODE83)
            return std::make_pair(ERR_PARAM, nullptr);

        auto result = std::make_shared<std::vector<DirEnumEntry>>();

        const uint8_t *p = romfs_start;
        while (1) {
            struct FileEntry *fe = (struct FileEntry *)p;
            if (fe->rec_size == 0)
                break;
            p += fe->rec_size;

            result->emplace_back(fe->filename, (uint32_t)fe->fsize, 0, (uint16_t)fe->fdate, (uint16_t)fe->ftime);
        }
        return std::make_pair(0, result);
    }

    int stat(const std::string &_path, struct stat *st) {
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