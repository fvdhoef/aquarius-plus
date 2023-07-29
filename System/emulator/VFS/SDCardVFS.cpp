#include "SDCardVFS.h"

#ifndef _WIN32
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <dirent.h>
#    include <unistd.h>
#else
#    include <io.h>
#    include <time.h>
#endif

#define MAX_FDS (10)

struct state {
    FILE *fds[MAX_FDS];
};

static struct state state;

SDCardVFS::SDCardVFS() {
}

SDCardVFS &SDCardVFS::instance() {
    static SDCardVFS vfs;
    return vfs;
}

void SDCardVFS::init(const std::string &_basePath) {
    basePath = _basePath;

    // // Copy basepath (without trailing slash if present)
    // size_t basepath_len = strlen(basepath);
    // while (basepath[basepath_len - 1] == '/' ||
    //        basepath[basepath_len - 1] == '\\') {
    //     basepath_len--;
    // }
    // state.basepath = (char *)malloc(basepath_len + 1);
    // assert(state.basepath != nullptr);
    // strncpy(state.basepath, basepath, basepath_len);
    // state.basepath[basepath_len] = 0;

    printf("basepath: '%s'\n", basePath.c_str());
}

std::string SDCardVFS::getFullPath(const std::string &path) {
    // Compose full path
    std::string result = basePath;
    result += "/";
    result += path;
    return result;
}

int SDCardVFS::open(uint8_t flags, const std::string &path) {
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
        if (state.fds[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    auto  fullPath = getFullPath(path);
    FILE *f        = ::fopen(fullPath.c_str(), mode);
    if (f == nullptr) {
        uint8_t err = ERR_NOT_FOUND;
        switch (errno) {
            case EACCES: err = ERR_NOT_FOUND; break;
            case EEXIST: err = ERR_EXISTS; break;
            default: err = ERR_NOT_FOUND; break;
        }
        return err;
    }
    state.fds[fd] = f;
    return fd;
}

int SDCardVFS::close(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    ::fclose(f);
    state.fds[fd] = nullptr;
    return 0;
}

int SDCardVFS::read(int fd, size_t size, void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::fread(buf, 1, size, f);
    return (result < 0) ? ERR_OTHER : result;
}

int SDCardVFS::write(int fd, size_t size, const void *buf) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = (int)::fwrite(buf, 1, size, f);
    return (result < 0) ? ERR_OTHER : result;
}

int SDCardVFS::seek(int fd, size_t offset) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = ::fseek(f, offset, SEEK_SET);
    return (result < 0) ? ERR_OTHER : 0;
}

int SDCardVFS::tell(int fd) {
    if (fd >= MAX_FDS || state.fds[fd] == nullptr)
        return ERR_PARAM;
    FILE *f = state.fds[fd];

    int result = ::ftell(f);
    return (result < 0) ? ERR_OTHER : result;
}

DirEnumCtx SDCardVFS::direnum(const std::string &path) {
    auto fullPath = getFullPath(path);

#ifndef _WIN32
    DIR *dir = opendir(fullPath.c_str());
    if (dir == nullptr) {
        return nullptr;
    }
#else
    struct _finddata_t fileinfo;
    intptr_t           handle = _findfirst((fullPath + "/*.*").c_str(), &fileinfo);
    if (handle < 0) {
        return nullptr;
    }
    bool first = true;
#endif

    auto result = std::make_shared<std::vector<DirEnumEntry>>();

    // Read directory contents
    while (1) {
        DirEnumEntry dee;
#ifndef _WIN32
        // Read directory entry
        struct dirent *de = readdir(dir);
        if (de == NULL) {
            break;
        }

        // Read additional file stats
        struct stat st;
        if (stat((fullPath + "/" + de->d_name).c_str(), &st) < 0) {
            continue;
        }

        // Return file entry
        dee.filename = de->d_name;
        dee.size     = (de->d_type == DT_DIR) ? 0 : st.st_size;
        dee.attr     = (de->d_type == DT_DIR) ? DE_DIR : 0;

#    ifdef __APPLE__
        time_t t = st.st_mtimespec.tv_sec;
#    else
        time_t t = st.st_mtim.tv_sec;
#    endif

        struct tm *tm = localtime(&t);
        dee.ftime     = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
        dee.fdate     = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

#else
        if (!first) {
            if (_findnext(handle, &fileinfo) != 0)
                break;
        }
        first = false;

        // Return file entry
        snprintf(dee->filename, sizeof(dee->filename), "%s", fileinfo.name);
        dee->size = (fileinfo.attrib & _A_SUBDIR) ? 0 : fileinfo.size;
        dee->attr = (fileinfo.attrib & _A_SUBDIR) ? DE_DIR : 0;
        time_t t  = fileinfo.time_write;

        struct tm *tm = localtime(&t);
        dee.ftime     = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
        dee.fdate     = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

        // Skip hidden and system files
        if (fileinfo.attrib & (_A_HIDDEN | _A_SYSTEM))
            continue;
#endif
        // Skip files starting with a dot
        if (dee.filename.size() == 0 || dee.filename[0] == '.')
            continue;

        result->push_back(dee);
    }

#ifndef _WIN32
    closedir(dir);
#else
    _findclose(handle);
#endif

    return result;
}

int SDCardVFS::delete_(const std::string &path) {
    auto fullPath = getFullPath(path);

    int result = ::unlink(fullPath.c_str());
    if (result < 0) {
        result = ::rmdir(fullPath.c_str());
    }

    if (result < 0) {
        // Error
        if (errno == ENOTEMPTY) {
            return ERR_NOT_EMPTY;
        } else {
            return ERR_NOT_FOUND;
        }
    }
    return 0;
}

int SDCardVFS::rename(const std::string &pathOld, const std::string &pathNew) {
    auto fullOld = getFullPath(pathOld);
    auto fullNew = getFullPath(pathNew);

    int result = ::rename(fullOld.c_str(), fullNew.c_str());
    return (result < 0) ? ERR_NOT_FOUND : 0;
}

int SDCardVFS::mkdir(const std::string &path) {
    auto fullPath = getFullPath(path);

    int result = ::mkdir(fullPath.c_str(), 0775);
    return (result < 0) ? ERR_OTHER : 0;
}

int SDCardVFS::stat(const std::string &path, struct stat *st) {
    auto fullPath = getFullPath(path);
    int  result   = ::stat(fullPath.c_str(), st);
    return result < 0 ? ERR_NOT_FOUND : 0;
}
