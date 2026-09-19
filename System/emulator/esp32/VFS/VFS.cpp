#include "VFS.h"
#include <algorithm>

#ifdef EMULATOR
#ifndef _WIN32
static inline std::string toUpper(std::string s) {
    for (auto &ch : s)
        ch = toupper(ch);
    return s;
}
#endif
#endif

VFSContext::VFSContext() {
    for (int i = 0; i < MAX_FDS; i++) {
        fdVfs[i] = nullptr;
        fds[i]   = 0;
    }
    for (int i = 0; i < MAX_DDS; i++) {
        deCtxs[i] = nullptr;
        deIdx[i]  = 0;
    }
}

std::string VFSContext::resolvePath(std::string path, VFS **vfs, std::string *wildCard) {
    *vfs = getSDCardVFS();

    if (startsWith(path, "http://") || startsWith(path, "https://")) {
        *vfs = getHttpVFS();
        return path;
    }
    if (startsWith(path, "tcp://")) {
        *vfs = getTcpVFS();
        return path;
    }

    bool useCwd = true;
    if (!path.empty() && (path[0] == '/' || path[0] == '\\')) {
        useCwd = false;
    } else if (startsWith(path, ESP_PREFIX)) {
        useCwd = false;
        *vfs   = getEspVFS();
        path   = path.substr(strlen(ESP_PREFIX));
    }

    // Split the path into parts
    std::vector<std::string> parts;
    if (useCwd) {
        if (startsWith(currentPath, ESP_PREFIX)) {
            splitPath(currentPath.substr(strlen(ESP_PREFIX)), parts);
            *vfs = getEspVFS();
        } else {
            splitPath(currentPath, parts);
        }
    }
    splitPath(path, parts);

    // Resolve path
    int idx = 0;
    while (idx < (int)parts.size()) {
        if (parts[idx] == ".") {
            parts.erase(parts.begin() + idx);
            continue;
        }
        if (parts[idx] == "..") {
            auto iterLast = parts.begin() + idx + 1;
            if (idx > 0)
                idx--;
            auto iterFirst = parts.begin() + idx;
            parts.erase(iterFirst, iterLast);
            continue;
        }
        idx++;
    }

    if (!parts.empty() && wildCard != nullptr) {
        const auto &lastPart = parts.back();
        if (lastPart.find_first_of("?*") != lastPart.npos) {
            // Contains wildcard, return it separately
            *wildCard = lastPart;
            parts.pop_back();
        }
    }

    // Compose resolved path
    std::string result;
    for (auto &part : parts) {

#ifdef EMULATOR
#ifndef _WIN32
        // Handle case-sensitive host file systems
        auto curPartUpper = toUpper(part);
        if (*vfs == getSDCardVFS()) {
            auto [deResult, deCtx] = (*vfs)->direnum(result, 0);
            if (deResult == 0) {
                for (auto &dee : *deCtx) {
                    if (toUpper(dee.filename) == curPartUpper) {
                        part = dee.filename;
                        break;
                    }
                }
            }
        }
#endif
#endif

        if (!result.empty())
            result += '/';
        result += part;
    }
    return result;
}

static bool wildcardMatch(const std::string &text, const std::string &pattern) {
    // Initialize the pointers to the current positions in the text and pattern strings.
    int textPos    = 0;
    int patternPos = 0;

    // Loop while we have not reached the end of either string.
    while (textPos < (int)text.size() && patternPos < (int)pattern.size()) {
        if (pattern[patternPos] == '*') {
            // Skip asterisk (and any following asterisks)
            while (patternPos < (int)pattern.size() && pattern[patternPos] == '*') {
                patternPos++;
            }
            // If end of the pattern then we have a match
            if (patternPos == (int)pattern.size()) {
                return true;
            }
            // Skip characters in text until we find one that matches the current pattern character
            while (tolower(text[textPos]) != tolower(pattern[patternPos])) {
                textPos++;

                // Reached end of text, but not end of pattern, no match
                if (textPos == (int)text.size())
                    return false;
            }
            continue;
        }

        // Check character match
        if (pattern[patternPos] != '?' && (tolower(text[textPos]) != tolower(pattern[patternPos])))
            return false;

        textPos++;
        patternPos++;
    }

    // If we reached the end of both strings, then the match is successful.
    return (textPos == (int)text.size() && patternPos == (int)pattern.size());
}

void VFSContext::reset() {
    closeAll();
    currentPath.clear();
}

void VFSContext::closeAll() {
    // Close any open descriptors
    for (int i = 0; i < MAX_FDS; i++) {
        if (fdVfs[i] != nullptr) {
            fdVfs[i]->close(fds[i]);
            fdVfs[i] = nullptr;
        }
    }
    for (int i = 0; i < MAX_DDS; i++) {
        deCtxs[i] = nullptr;
    }

#ifdef EMULATOR
    fi.clear();
    di.clear();
#endif
}

int VFSContext::open(uint8_t flags, const std::string &pathArg) {
    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (fdVfs[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1)
        return ERR_TOO_MANY_OPEN;

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return ERR_PARAM;

    int vfs_fd = vfs->open(flags, path);
    if (vfs_fd < 0) {
        return vfs_fd;
    } else {
        fdVfs[fd] = vfs;
        fds[fd]   = vfs_fd;

#ifdef EMULATOR
        FileInfo tmp;
        tmp.flags  = flags;
        tmp.name   = pathArg;
        tmp.offset = 0;
        fi.insert(std::make_pair(fd, tmp));
#endif
        return fd;
    }
}

int VFSContext::close(int fd) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->close(fds[fd]);
    fdVfs[fd]  = nullptr;

#ifdef EMULATOR
    auto it = fi.find(fd);
    if (it != fi.end()) {
        fi.erase(it);
    }
#endif
    return result;
}

int VFSContext::read(int fd, size_t size, void *buf) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->read(fds[fd], size, buf);
#ifdef EMULATOR
    if (result >= 0) {
        fi[fd].offset += result;
    }
#endif
    return result;
}

int VFSContext::readline(int fd, size_t size, void *buf) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->readline(fds[fd], size, buf);
#ifdef EMULATOR
    fi[fd].offset = fdVfs[fd]->tell(fds[fd]);
#endif
    return result;
}

int VFSContext::write(int fd, size_t size, const void *buf) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->write(fds[fd], size, buf);
#ifdef EMULATOR
    if (result >= 0) {
        fi[fd].offset += result;
    }
#endif
    return result;
}

int VFSContext::seek(int fd, size_t offset) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->seek(fds[fd], offset);
#ifdef EMULATOR
    fi[fd].offset = fdVfs[fd]->tell(fds[fd]);
#endif
    return result;
}

int VFSContext::lseek(int fd, int offset, int whence) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;

    int result = fdVfs[fd]->lseek(fds[fd], offset, whence);
#ifdef EMULATOR
    fi[fd].offset = fdVfs[fd]->tell(fds[fd]);
#endif
    return result;
}

int VFSContext::tell(int fd) {
    if (fd >= MAX_FDS || fdVfs[fd] == nullptr)
        return ERR_PARAM;
    int result = fdVfs[fd]->tell(fds[fd]);
    return result;
}

int VFSContext::openDirExt(const char *pathArg, uint8_t flags, uint16_t skipCount) {
    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (deCtxs[i] == nullptr) {
            dd = i;
            break;
        }
    }
    if (dd == -1)
        return ERR_TOO_MANY_OPEN;

    // Compose full path
    VFS        *vfs = nullptr;
    std::string wildCard;
    auto        path = resolvePath(pathArg, &vfs, &wildCard);
    if (!vfs)
        return ERR_PARAM;

    auto [result, deCtx] = vfs->direnum(path, flags);
    if (result < 0)
        return result;

    if (!path.empty() && (flags & DE_FLAG_DOTDOT) != 0)
        deCtx->push_back(DirEnumEntry("..", 0, DE_ATTR_DIR, 0, 0));

    if (!wildCard.empty())
        deCtx->erase(
            std::remove_if(deCtx->begin(), deCtx->end(), [&](DirEnumEntry &de) {
                if ((de.attr & DE_ATTR_DIR) != 0 && (flags & DE_FLAG_ALWAYS_DIRS))
                    return false;
                return !wildcardMatch(de.filename, wildCard);
            }),
            deCtx->end());

    std::sort(deCtx->begin(), deCtx->end(), [](auto &a, auto &b) {
        // Sort directories at the top
        if ((a.attr & DE_ATTR_DIR) != (b.attr & DE_ATTR_DIR))
            return (a.attr & DE_ATTR_DIR) != 0;
        return strcasecmp(a.filename.c_str(), b.filename.c_str()) < 0;
    });

    deCtxs[dd] = deCtx;
    deIdx[dd]  = skipCount;

#ifdef EMULATOR
    DirInfo tmp;
    tmp.name   = pathArg;
    tmp.offset = 0;
    di.insert(std::make_pair(dd, tmp));
#endif
    return dd;
}

int VFSContext::closeDir(int dd) {
    if (dd >= MAX_DDS || deCtxs[dd] == nullptr)
        return ERR_PARAM;
    deCtxs[dd] = nullptr;

#ifdef EMULATOR
    auto it = di.find(dd);
    if (it != di.end()) {
        di.erase(it);
    }
#endif
    return 0;
}

int VFSContext::readDir(int dd, DirEnumEntry *de) {
    if (dd >= MAX_DDS || deCtxs[dd] == nullptr)
        return ERR_PARAM;

    auto ctx = deCtxs[dd];
    if (deIdx[dd] >= (int)((*ctx).size()))
        return ERR_EOF;

    *de = (*ctx)[deIdx[dd]++];

#ifdef EMULATOR
    di[dd].offset++;
#endif
    return 0;
}

int VFSContext::delete_(const std::string &pathArg) {
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return ERR_PARAM;
    return vfs->delete_(path);
}

int VFSContext::rename(const std::string &pathOld, const std::string &pathNew) {
    VFS *vfs1     = nullptr;
    VFS *vfs2     = nullptr;
    auto _oldPath = resolvePath(pathOld, &vfs1);
    auto _newPath = resolvePath(pathNew, &vfs2);
    if (!vfs1 || vfs1 != vfs2)
        return ERR_PARAM;
    return vfs1->rename(_oldPath, _newPath);
}

int VFSContext::mkdir(const std::string &pathArg) {
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return ERR_PARAM;
    return vfs->mkdir(path);
}

int VFSContext::chdir(const std::string &pathArg) {
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return ERR_PARAM;

    struct stat st;
    int         result = vfs->stat(path, &st);
    if (result == 0) {
        if (st.st_mode & S_IFDIR) {
            if (vfs == getEspVFS()) {
                currentPath = std::string(ESP_PREFIX) + path;
            } else {
                currentPath = path;
            }
        } else {
            result = ERR_PARAM;
        }
    }
    return result;
}

int VFSContext::stat(const std::string &pathArg, struct stat *st) {
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return ERR_PARAM;

    return vfs->stat(path, st);
}

std::pair<int, std::vector<uint8_t>> VFSContext::readFile(const std::string &pathArg, bool zeroTerminate) {
    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs)
        return {ERR_PARAM, {}};

    struct stat st;
    int         res = vfs->stat(path, &st);
    if (res < 0 || (st.st_mode & S_IFREG) == 0)
        return {res, {}};

    unsigned fileSize = st.st_size;

    std::vector<uint8_t> data;
    data.resize(fileSize + (zeroTerminate ? 1 : 0));

    int fd = vfs->open(FO_RDONLY, path);
    if (fd < 0)
        return {fd, {}};
    vfs->read(fd, st.st_size, data.data());
    vfs->close(fd);

    if (zeroTerminate)
        data[fileSize] = 0;

    return {0, data};
}

VFSContext *VFSContext::getDefault() {
    static VFSContext obj;
    return &obj;
}
