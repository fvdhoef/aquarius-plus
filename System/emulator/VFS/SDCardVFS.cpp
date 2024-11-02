#include "VFS.h"

#ifndef _WIN32
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#else
#include <io.h>
#include <time.h>
#endif

#define MAX_FDS (10)

class SDCardVFS : public VFS {
public:
    std::string basePath;
    FILE       *fds[MAX_FDS];

    SDCardVFS() {
    }

    void setBasePath(const std::string &_basePath) {
        // printf("Settings SD card directory: '%s'\n", _basePath.c_str());

        basePath = _basePath;
        stripTrailingSlashes(basePath);
    }

    std::string getFullPath(const std::string &path) {
        // Compose full path
        std::string result = basePath;
        result += "/";
        result += path;
        stripTrailingSlashes(result);
        return result;
    }

    static int mapErrNoResult(void) {
        switch (errno) {
            case EEXIST: return ERR_EXISTS;
            case EACCES:
            case ENOENT: return ERR_NOT_FOUND;
            case EINVAL:
            case EBADF: return ERR_PARAM;
            case ENOTEMPTY: return ERR_NOT_EMPTY;
        }
        return ERR_OTHER;
    }

    int open(uint8_t flags, const std::string &path) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        // Translate flags
        int  mi = 0;
        char mode[5];

        // if (flags & FO_CREATE)
        //     oflag |= O_CREAT;

        switch (flags & FO_ACCMODE) {
            case FO_RDONLY:
                mode[mi++] = 'r';
                break;
            case FO_WRONLY:
                if (flags & FO_APPEND) {
                    mode[mi++] = 'a';
                } else {
                    mode[mi++] = 'w';
                }
                if (flags & FO_EXCL) {
                    mode[mi++] = 'x';
                }

                break;
            case FO_RDWR:
                if (flags & FO_APPEND) {
                    mode[mi++] = 'a';
                    mode[mi++] = '+';
                } else if (flags & FO_TRUNC) {
                    mode[mi++] = 'w';
                    mode[mi++] = '+';
                } else {
                    mode[mi++] = 'r';
                    mode[mi++] = '+';
                }
                if (flags & FO_EXCL) {
                    mode[mi++] = 'x';
                }
                break;

            default: {
                // Error
                return ERR_PARAM;
            }
        }
        mode[mi++] = 'b';
        mode[mi]   = 0;

        // Find free file descriptor
        int fd = -1;
        for (int i = 0; i < MAX_FDS; i++) {
            if (fds[i] == nullptr) {
                fd = i;
                break;
            }
        }
        if (fd == -1)
            return ERR_TOO_MANY_OPEN;

        auto  fullPath = getFullPath(path);
        FILE *f        = ::fopen(fullPath.c_str(), mode);
        if (f == nullptr) {
            return mapErrNoResult();
        }
        fds[fd] = f;
        return fd;
    }

    int close(int fd) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;
        FILE *f = fds[fd];

        ::fclose(f);
        fds[fd] = nullptr;
        return 0;
    }

    int read(int fd, size_t size, void *buf) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;
        FILE *f = fds[fd];

        int result = (int)::fread(buf, 1, size, f);
        return (result < 0) ? mapErrNoResult() : result;
    }

    int readline(int fd, size_t size, void *buf) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;

        FILE *f = fds[fd];
        if (fgets((char *)buf, (int)size, f) == NULL) {
            if (feof(f))
                return ERR_EOF;
            errno = ferror(f);
            return mapErrNoResult();
        }
        return 0;
    }

    int write(int fd, size_t size, const void *buf) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;
        FILE *f = fds[fd];

        int result = (int)::fwrite(buf, 1, size, f);
        return (result < 0) ? mapErrNoResult() : result;
    }

    int seek(int fd, size_t offset) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;
        FILE *f = fds[fd];

        int result = (int)::fseek(f, (long)offset, SEEK_SET);
        return (result < 0) ? mapErrNoResult() : 0;
    }

    int tell(int fd) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        if (fd >= MAX_FDS || fds[fd] == nullptr)
            return ERR_PARAM;
        FILE *f = fds[fd];

        int result = (int)::ftell(f);
        return (result < 0) ? mapErrNoResult() : result;
    }

    std::pair<int, DirEnumCtx> direnum(const std::string &path, uint8_t flags) override {
        if (basePath.empty())
            return std::pair(ERR_NO_DISK, nullptr);

        bool mode83     = (flags & DE_FLAG_MODE83) != 0;
        bool showHidden = (flags & DE_FLAG_HIDDEN) != 0;

        auto fullPath = getFullPath(path);

#ifndef _WIN32
        DIR *dir = ::opendir(fullPath.c_str());
        if (dir == nullptr) {
            return std::pair(ERR_NOT_FOUND, nullptr);
        }
#else
        struct _finddata_t fileinfo;
        intptr_t           handle = _findfirst((fullPath + "/*.*").c_str(), &fileinfo);
        if (handle < 0) {
            return std::pair(ERR_NOT_FOUND, nullptr);
        }
        bool first = true;
#endif

        auto result = std::make_shared<std::vector<DirEnumEntry>>();

        // Read directory contents
        while (1) {
            DirEnumEntry dee;
#ifndef _WIN32
            // Read directory entry
            struct dirent *de = ::readdir(dir);
            if (de == NULL) {
                break;
            }

            // Read additional file stats
            std::string filePath = fullPath + "/" + de->d_name;
            struct stat st;
            if (::stat(filePath.c_str(), &st) < 0) {
                continue;
            }

            // Return file entry
            dee.filename = de->d_name;
            dee.size     = (de->d_type == DT_DIR) ? 0 : st.st_size;
            dee.attr     = (de->d_type == DT_DIR) ? DE_ATTR_DIR : 0;

#ifdef __APPLE__
            time_t t = st.st_mtimespec.tv_sec;
#else
            time_t t = st.st_mtim.tv_sec;
#endif

#else
            if (!first) {
                if (_findnext(handle, &fileinfo) != 0)
                    break;
            }
            first = false;

            // Skip hidden and system files
            if (fileinfo.attrib & (_A_HIDDEN | _A_SYSTEM))
                continue;

            // Return file entry
            dee.filename = fileinfo.name;
            dee.size     = (fileinfo.attrib & _A_SUBDIR) ? 0 : fileinfo.size;
            dee.attr     = (fileinfo.attrib & _A_SUBDIR) ? DE_ATTR_DIR : 0;
            time_t t     = fileinfo.time_write;
#endif

            struct tm *tm = ::localtime(&t);
            dee.ftime     = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
            dee.fdate     = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

            // Skip files starting with a dot
            if (dee.filename.size() == 0 || dee.filename == "." || dee.filename == ".." || (dee.filename[0] == '.' && !showHidden))
                continue;

            if (mode83) {
                // Skip file if it does not conform to 8.3

                // Filename length > 12 characters?
                if (dee.filename.size() > 12)
                    continue;

                auto extPos = dee.filename.find_first_of('.');
                if (extPos == std::string::npos) {
                    // No extension and length > 8?
                    if (dee.filename.size() > 8)
                        continue;

                } else {
                    // Base filename > 8 characters
                    if (extPos > 8) {
                        continue;
                    }
                    // Multiple dots?
                    if (dee.filename.find_first_of('.', extPos + 1) != std::string::npos)
                        continue;
                }

                // Disallowed characters?
                bool ok = true;
                for (unsigned i = 0; i < dee.filename.size(); i++) {
                    dee.filename[i] = std::toupper(dee.filename[i]);
                    if (dee.filename[i] <= ' ' || dee.filename[i] >= '~') {
                        ok = false;
                        break;
                    }
                }
                if (!ok)
                    continue;
            }

            result->push_back(dee);
        }

#ifndef _WIN32
        ::closedir(dir);
#else
        ::_findclose(handle);
#endif

        return std::make_pair(0, result);
    }

    int delete_(const std::string &path) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        auto fullPath = getFullPath(path);

        int result = ::unlink(fullPath.c_str());
        if (result < 0) {
            result = ::rmdir(fullPath.c_str());
        }
        return (result < 0) ? mapErrNoResult() : 0;
    }

    int rename(const std::string &pathOld, const std::string &pathNew) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        auto fullOld = getFullPath(pathOld);
        auto fullNew = getFullPath(pathNew);

        int result = ::rename(fullOld.c_str(), fullNew.c_str());
        return (result < 0) ? mapErrNoResult() : 0;
    }

    int mkdir(const std::string &path) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        auto fullPath = getFullPath(path);

#if _WIN32
        int result = ::mkdir(fullPath.c_str());
#else
        int result = ::mkdir(fullPath.c_str(), 0775);
#endif
        return (result < 0) ? mapErrNoResult() : 0;
    }

    int stat(const std::string &path, struct stat *st) override {
        if (basePath.empty())
            return ERR_NO_DISK;

        auto fullPath = getFullPath(path);
        int  result   = ::stat(fullPath.c_str(), st);
        return (result < 0) ? mapErrNoResult() : 0;
    }
};

VFS *getSDCardVFS() {
    static SDCardVFS obj;
    return &obj;
}

void setSDCardPath(const std::string &basePath) {
    static_cast<SDCardVFS *>(getSDCardVFS())->setBasePath(basePath);
}
