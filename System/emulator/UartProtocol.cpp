#include "UartProtocol.h"
#include "VFS.h"
#include "Keyboard.h"
#include <algorithm>
#include <time.h>
#include <math.h>
#include "EmuState.h"
#include "Config.h"

enum {
    ESPCMD_RESET             = 0x01, // Indicate to ESP that system has been reset
    ESPCMD_VERSION           = 0x02, // Get version string
    ESPCMD_GETDATETIME       = 0x03, // Get current date/time
    ESPCMD_KEYMODE           = 0x08, // Set keyboard mode
    ESPCMD_GETMOUSE          = 0x0C, // Get mouse state
    ESPCMD_GETGAMECTRL       = 0x0E, // Get game controller state
    ESPCMD_OPEN              = 0x10, // Open / create file
    ESPCMD_CLOSE             = 0x11, // Close open file
    ESPCMD_READ              = 0x12, // Read from file
    ESPCMD_WRITE             = 0x13, // Write to file
    ESPCMD_SEEK              = 0x14, // Move read/write pointer
    ESPCMD_TELL              = 0x15, // Get current read/write
    ESPCMD_OPENDIR           = 0x16, // Open directory
    ESPCMD_CLOSEDIR          = 0x17, // Close open directory
    ESPCMD_READDIR           = 0x18, // Read from directory
    ESPCMD_DELETE            = 0x19, // Remove file or directory
    ESPCMD_RENAME            = 0x1A, // Rename / move file or directory
    ESPCMD_MKDIR             = 0x1B, // Create directory
    ESPCMD_CHDIR             = 0x1C, // Change directory
    ESPCMD_STAT              = 0x1D, // Get file status
    ESPCMD_GETCWD            = 0x1E, // Get current working directory
    ESPCMD_CLOSEALL          = 0x1F, // Close any open file/directory descriptor
    ESPCMD_OPENDIR83         = 0x20, // Open directory in 8.3 mode
    ESPCMD_READLINE          = 0x21, // Read line from file
    ESPCMD_OPENDIREXT        = 0x22, // Open directory with extended options
    ESPCMD_LOADFPGA          = 0x40, // Load FPGA bitstream
    ESPCMD_WIFI_STATUS       = 0x50, // Get WiFi status
    ESPCMD_WIFI_SCAN         = 0x51, // Scan for WiFi APs
    ESPCMD_WIFI_CONFIG       = 0x52, // Configure WiFi
    ESPCMD_WIFI_SET_HOSTNAME = 0x53, // Set hostname
};

#define ESP_PREFIX "esp:"

#if 0
#define DBGF(...) printf(__VA_ARGS__)
#else
#define DBGF(...)
#endif

UartProtocol::UartProtocol() {
    rxBufIdx = 0;
    for (int i = 0; i < MAX_FDS; i++) {
        fdVfs[i] = nullptr;
        fds[i]   = 0;
    }
    for (int i = 0; i < MAX_DDS; i++) {
        deCtxs[i] = nullptr;
        deIdx[i]  = 0;
    }
}

UartProtocol *UartProtocol::instance() {
    static UartProtocol *obj = nullptr;
    if (obj == nullptr)
        obj = new UartProtocol();
    return obj;
}

void UartProtocol::init() {
    getEspVFS()->init();
    getHttpVFS()->init();
    getTcpVFS()->init();
}

void UartProtocol::writeData(uint8_t data) {
    receivedByte(data);
}

void UartProtocol::writeCtrl(uint8_t data) {
    if (data & 0x80) {
        rxBufIdx = 0;
    }
}

uint8_t UartProtocol::readData() {
    int data = txFifoRead();
    if (data < 0) {
        printf("esp32_read_data - Empty!\n");
        return 0;
    }
    return data;
}

uint8_t UartProtocol::readCtrl() {
    uint8_t result = 0;
    if (txBufCnt > 0) {
        result |= 1;
    }
    return result;
}

int UartProtocol::txFifoRead() {
    int result = -1;
    if (txBufCnt > 0) {
        result = txBuf[txBufRdIdx++];
        txBufCnt--;
        if (txBufRdIdx >= sizeof(txBuf)) {
            txBufRdIdx = 0;
        }
    }
    return result;
}

void UartProtocol::txFifoWrite(uint8_t data) {
    if (txBufCnt >= sizeof(txBuf)) {
        return;
    }

    txBuf[txBufWrIdx++] = data;
    txBufCnt++;
    if (txBufWrIdx >= sizeof(txBuf)) {
        txBufWrIdx = 0;
    }
}

void UartProtocol::txFifoWrite(const void *buf, size_t length) {
    auto p = (const uint8_t *)buf;
    while (length--) {
        txFifoWrite(*(p++));
    }
}

void UartProtocol::splitPath(const std::string &path, std::vector<std::string> &result) {
    const char *delimiters = "/\\";
    size_t      start;
    size_t      end = 0;
    while ((start = path.find_first_not_of(delimiters, end)) != std::string::npos) {
        end = path.find_first_of(delimiters, start);
        result.push_back(path.substr(start, end - start));
    }
}

static bool startsWith(const std::string &s1, const std::string &s2, bool caseSensitive = false) {
    if (caseSensitive) {
        return (strncmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    } else {
        return (strncasecmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    }
}

#ifndef _WIN32
static inline std::string toUpper(std::string s) {
    for (auto &ch : s)
        ch = toupper(ch);
    return s;
}
#endif

std::string UartProtocol::resolvePath(std::string path, VFS **vfs, std::string *wildCard) {
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

        if (!result.empty())
            result += '/';
        result += part;
    }
    return result;
}

void UartProtocol::closeAllDescriptors() {
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
}

void UartProtocol::receivedByte(uint8_t data) {
    if (rxBufIdx == 0 && data == 0) {
        // Ignore zero-bytes after break
        return;
    }

    // printf("UartProtocol::receivedByte %02X\n", data);

    rxBuf[rxBufIdx] = data;
    if (rxBufIdx < sizeof(rxBuf) - 1) {
        rxBufIdx++;
    }

    if (rxBufIdx > 0) {
        uint8_t cmd = rxBuf[0];

        switch (cmd) {
            case ESPCMD_RESET: {
                // Close any open descriptors
                DBGF("RESET\n");
                cmdReset();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_VERSION: {
                cmdVersion();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_GETDATETIME: {
                if (rxBufIdx == 2) {
                    uint8_t type = rxBuf[1];
                    cmdGetDateTime(type);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_KEYMODE: {
                if (rxBufIdx == 2) {
                    uint8_t keyMode = rxBuf[1];
                    cmdKeyMode(keyMode);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_GETMOUSE: {
                cmdGetMouse();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_GETGAMECTRL: {
                if (rxBufIdx == 2) {
                    uint8_t ctrlIdx = rxBuf[1];
                    cmdGetGameCtrl(ctrlIdx);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_OPEN: {
                if (data == 0 && rxBufIdx >= 3) {
                    uint8_t     flags   = rxBuf[1];
                    const char *pathArg = (const char *)&rxBuf[2];
                    cmdOpen(flags, pathArg);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CLOSE: {
                if (rxBufIdx == 2) {
                    uint8_t fd = rxBuf[1];
                    cmdClose(fd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_READ: {
                if (rxBufIdx == 4) {
                    uint8_t  fd   = rxBuf[1];
                    uint16_t size = rxBuf[2] | (rxBuf[3] << 8);
                    cmdRead(fd, size);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_WRITE: {
                if (rxBufIdx >= 4) {
                    uint8_t     fd   = rxBuf[1];
                    unsigned    size = rxBuf[2] | (rxBuf[3] << 8);
                    const void *buf  = &rxBuf[4];
                    if (rxBufIdx == 4 + size) {
                        cmdWrite(fd, size, buf);
                        rxBufIdx = 0;
                    }
                }
                break;
            }
            case ESPCMD_SEEK: {
                if (rxBufIdx == 6) {
                    uint8_t  fd     = rxBuf[1];
                    uint32_t offset = (rxBuf[2] << 0) | (rxBuf[3] << 8) | (rxBuf[4] << 16) | (rxBuf[5] << 24);
                    cmdSeek(fd, offset);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_TELL: {
                if (rxBufIdx == 2) {
                    uint8_t fd = rxBuf[1];
                    cmdTell(fd);
                    rxBufIdx = 0;
                }
                break;
            }

            case ESPCMD_OPENDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdOpenDirExt(pathArg, 0, 0);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_OPENDIR83: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdOpenDirExt(pathArg, DE_FLAG_MODE83, 0);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_OPENDIREXT: {
                if (data == 0 && rxBufIdx >= 5) {
                    uint8_t     flags      = rxBuf[1];
                    uint16_t    skip_count = rxBuf[2] | (rxBuf[3] << 8);
                    const char *pathArg    = (const char *)&rxBuf[4];
                    cmdOpenDirExt(pathArg, flags, skip_count);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CLOSEDIR: {
                if (rxBufIdx == 2) {
                    uint8_t dd = rxBuf[1];
                    cmdCloseDir(dd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_READDIR: {
                if (rxBufIdx == 2) {
                    uint8_t dd = rxBuf[1];
                    cmdReadDir(dd);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_DELETE: {
                // Wait for zero-termination of path
                if (data == 0) {
                    const char *pathArg = (const char *)&rxBuf[1];
                    cmdDelete(pathArg);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_RENAME: {
                if (rxBufIdx == 1) {
                    newPath = nullptr;
                }

                if (data == 0) {
                    const char *oldPath = (const char *)&rxBuf[1];
                    if (newPath == nullptr) {
                        newPath = (const char *)&rxBuf[rxBufIdx];
                    } else {
                        cmdRename(oldPath, newPath);
                        newPath  = nullptr;
                        rxBufIdx = 0;
                    }
                }
                break;
            }
            case ESPCMD_MKDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdMkDir((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_CHDIR: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdChDir((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_STAT: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdStat((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_GETCWD: {
                cmdGetCwd();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_CLOSEALL: {
                cmdCloseAll();
                rxBufIdx = 0;
                break;
            }
            case ESPCMD_READLINE: {
                if (rxBufIdx == 4) {
                    uint8_t  fd   = rxBuf[1];
                    uint16_t size = rxBuf[2] | (rxBuf[3] << 8);
                    cmdReadLine(fd, size);
                    rxBufIdx = 0;
                }
                break;
            }
            case ESPCMD_LOADFPGA: {
                // Wait for zero-termination of path
                if (data == 0) {
                    cmdLoadFpga((const char *)&rxBuf[1]);
                    rxBufIdx = 0;
                }
                break;
            }
            default: {
                DBGF("Invalid command: 0x%02X\n", cmd);
                rxBufIdx = 0;
                break;
            }
        }
    }
}

void UartProtocol::cmdReset() {
    closeAllDescriptors();
    currentPath.clear();
}

void UartProtocol::cmdVersion() {
    DBGF("VERSION");

    extern const char *versionStr;
    const char        *p = versionStr;
    while (*p) {
        txFifoWrite(*(p++));
    }
    txFifoWrite(0);
}

void UartProtocol::cmdGetDateTime(uint8_t type) {
    DBGF("GETDATETIME");
    if (type != 0) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    time_t now;
    time(&now);
    struct tm timeinfo = *localtime(&now);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y%m%d%H%M%S", &timeinfo);

    txFifoWrite(0);

    const char *p = strftime_buf;
    while (*p) {
        txFifoWrite(*(p++));
    }
    txFifoWrite(0);
    return;
}

void UartProtocol::cmdKeyMode(uint8_t mode) {
    setKeyMode(mode);
    txFifoWrite(0);
    return;
}

void UartProtocol::cmdGetMouse() {
    DBGF("GETMOUSE");
    mousePresent = Config::instance()->enableMouse;

    if (!mousePresent) {
        txFifoWrite(ERR_NOT_FOUND);
        return;
    }
    txFifoWrite(0);

    uint16_t x = (uint16_t)mouseX;
    uint8_t  y = (uint8_t)mouseY;
    txFifoWrite(x & 0xFF);
    txFifoWrite(x >> 8);
    txFifoWrite(y);
    txFifoWrite(mouseButtons);
    txFifoWrite((int8_t)std::max(-128, std::min(mouseWheel, 127)));
    mouseWheel = 0;

    emuState.mouseHideTimeout = 1.0f;
}

void UartProtocol::cmdGetGameCtrl(uint8_t idx) {
    DBGF("GETGAMECTRL");
    if (idx != 0 || !gameCtrlPresent) {
        txFifoWrite(ERR_NOT_FOUND);
        return;
    }
    txFifoWrite(0);
    txFifoWrite(gameCtrlLX);
    txFifoWrite(gameCtrlLY);
    txFifoWrite(gameCtrlRX);
    txFifoWrite(gameCtrlRY);
    txFifoWrite(gameCtrlLT);
    txFifoWrite(gameCtrlRT);
    txFifoWrite(gameCtrlButtons & 0xFF);
    txFifoWrite(gameCtrlButtons >> 8);
}

void UartProtocol::cmdOpen(uint8_t flags, const char *pathArg) {
    DBGF("OPEN: flags: 0x%02X '%s'\n", flags, pathArg);

    // Find free file descriptor
    int fd = -1;
    for (int i = 0; i < MAX_FDS; i++) {
        if (fdVfs[i] == nullptr) {
            fd = i;
            break;
        }
    }
    if (fd == -1) {
        // Error
        txFifoWrite(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int vfs_fd = vfs->open(flags, path);
    if (vfs_fd < 0) {
        txFifoWrite(vfs_fd);
    } else {
        fdVfs[fd] = vfs;
        fds[fd]   = vfs_fd;
        txFifoWrite(fd);

        FileInfo tmp;
        tmp.flags  = flags;
        tmp.name   = pathArg;
        tmp.offset = 0;
        fi.insert(std::make_pair(fd, tmp));
    }
}

void UartProtocol::cmdClose(uint8_t fd) {
    DBGF("CLOSE fd: %d\n", fd);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(fdVfs[fd]->close(fds[fd]));
    fdVfs[fd] = nullptr;

    auto it = fi.find(fd);
    if (it != fi.end()) {
        fi.erase(it);
    }
}

void UartProtocol::cmdRead(uint8_t fd, uint16_t size) {
    DBGF("READ fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->read(fds[fd], size, rxBuf);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);
        txFifoWrite(rxBuf, result);

        fi[fd].offset += result;
    }
}

void UartProtocol::cmdReadLine(uint8_t fd, uint16_t size) {
    DBGF("READLINE fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->readline(fds[fd], size, rxBuf);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);

        const uint8_t *p = rxBuf;
        while (*p) {
            if (*p == '\r' || *p == '\n')
                break;
            txFifoWrite(*(p++));
        }
        txFifoWrite(0);

        fi[fd].offset = fdVfs[fd]->tell(fds[fd]);
    }
}

void UartProtocol::cmdWrite(uint8_t fd, uint16_t size, const void *data) {
    DBGF("WRITE fd: %d  size: %u\n", fd, size);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->write(fds[fd], size, data);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);

        fi[fd].offset += result;
    }
}

void UartProtocol::cmdSeek(uint8_t fd, uint32_t offset) {
    DBGF("SEEK fd: %d  offset: %u\n", fd, (unsigned)offset);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(fdVfs[fd]->seek(fds[fd], offset));

    fi[fd].offset = fdVfs[fd]->tell(fds[fd]);
}

void UartProtocol::cmdTell(uint8_t fd) {
    DBGF("TELL fd: %d\n", fd);

    if (fd >= MAX_FDS || fdVfs[fd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    int result = fdVfs[fd]->tell(fds[fd]);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        txFifoWrite(0);
        txFifoWrite((result >> 0) & 0xFF);
        txFifoWrite((result >> 8) & 0xFF);
        txFifoWrite((result >> 16) & 0xFF);
        txFifoWrite((result >> 24) & 0xFF);
    }
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

void UartProtocol::cmdOpenDirExt(const char *pathArg, uint8_t flags, uint16_t skip_count) {
    DBGF("OPENDIR: '%s'\n", pathArg);

    // Find free directory descriptor
    int dd = -1;
    for (int i = 0; i < MAX_DDS; i++) {
        if (deCtxs[i] == nullptr) {
            dd = i;
            break;
        }
    }
    if (dd == -1) {
        // Error
        txFifoWrite(ERR_TOO_MANY_OPEN);
        return;
    }

    // Compose full path
    VFS        *vfs = nullptr;
    std::string wildCard;
    auto        path = resolvePath(pathArg, &vfs, &wildCard);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    auto [result, deCtx] = vfs->direnum(path, flags);
    if (result < 0) {
        txFifoWrite(result);
    } else {
        if (!path.empty() && (flags & DE_FLAG_DOTDOT) != 0) {
            deCtx->push_back(DirEnumEntry("..", 0, DE_ATTR_DIR, 0, 0));
        }

        if (!wildCard.empty()) {
            deCtx->erase(
                std::remove_if(deCtx->begin(), deCtx->end(), [&](DirEnumEntry &de) {
                    if ((de.attr & DE_ATTR_DIR) != 0 && (flags & DE_FLAG_ALWAYS_DIRS))
                        return false;
                    return !wildcardMatch(de.filename, wildCard);
                }),
                deCtx->end());
        }

        std::sort(deCtx->begin(), deCtx->end(), [](auto &a, auto &b) {
            // Sort directories at the top
            if ((a.attr & DE_ATTR_DIR) != (b.attr & DE_ATTR_DIR)) {
                return (a.attr & DE_ATTR_DIR) != 0;
            }
            return strcasecmp(a.filename.c_str(), b.filename.c_str()) < 0;
        });

        deCtxs[dd] = deCtx;
        deIdx[dd]  = skip_count;
        txFifoWrite(dd);

        DirInfo tmp;
        tmp.name   = pathArg;
        tmp.offset = 0;
        di.insert(std::make_pair(dd, tmp));
    }
}

void UartProtocol::cmdCloseDir(uint8_t dd) {
    DBGF("CLOSEDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    deCtxs[dd] = nullptr;
    txFifoWrite(0);

    auto it = di.find(dd);
    if (it != di.end()) {
        di.erase(it);
    }
}

void UartProtocol::cmdReadDir(uint8_t dd) {
    DBGF("READDIR dd: %d\n", dd);

    if (dd >= MAX_DDS || deCtxs[dd] == nullptr) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    auto ctx = deCtxs[dd];
    if (deIdx[dd] >= (int)((*ctx).size())) {
        txFifoWrite(ERR_EOF);
        return;
    }

    auto &de = (*ctx)[deIdx[dd]++];
    txFifoWrite(0);
    txFifoWrite((de.fdate >> 0) & 0xFF);
    txFifoWrite((de.fdate >> 8) & 0xFF);
    txFifoWrite((de.ftime >> 0) & 0xFF);
    txFifoWrite((de.ftime >> 8) & 0xFF);
    txFifoWrite(de.attr);
    txFifoWrite((de.size >> 0) & 0xFF);
    txFifoWrite((de.size >> 8) & 0xFF);
    txFifoWrite((de.size >> 16) & 0xFF);
    txFifoWrite((de.size >> 24) & 0xFF);
    txFifoWrite(de.filename.c_str(), de.filename.size());
    txFifoWrite(0);

    di[dd].offset++;
}

void UartProtocol::cmdDelete(const char *pathArg) {
    DBGF("DELETE %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    txFifoWrite(vfs->delete_(path));
}

void UartProtocol::cmdRename(const char *oldArg, const char *newArg) {
    DBGF("RENAME %s -> %s\n", oldArg, newArg);

    VFS *vfs1     = nullptr;
    VFS *vfs2     = nullptr;
    auto _oldPath = resolvePath(oldArg, &vfs1);
    auto _newPath = resolvePath(newArg, &vfs2);
    if (!vfs1 || vfs1 != vfs2) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    txFifoWrite(vfs1->rename(_oldPath, _newPath));
}

void UartProtocol::cmdMkDir(const char *pathArg) {
    DBGF("MKDIR %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }
    txFifoWrite(vfs->mkdir(path));
}

void UartProtocol::cmdChDir(const char *pathArg) {
    DBGF("CHDIR %s\n", pathArg);

    // Compose full path
    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

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
    txFifoWrite(result);
}

void UartProtocol::cmdStat(const char *pathArg) {
    DBGF("STAT %s\n", pathArg);

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(path, &st);

    txFifoWrite(result);
    if (result < 0)
        return;

#ifdef __APPLE__
    time_t t = st.st_mtimespec.tv_sec;
#elif _WIN32
    time_t t = st.st_mtime;
#else
    time_t t = st.st_mtim.tv_sec;
#endif

    struct tm *tm       = localtime(&t);
    uint16_t   fat_time = (tm->tm_hour << 11) | (tm->tm_min << 5) | (tm->tm_sec / 2);
    uint16_t   fat_date = ((tm->tm_year + 1900 - 1980) << 9) | ((tm->tm_mon + 1) << 5) | tm->tm_mday;

    txFifoWrite((fat_date >> 0) & 0xFF);
    txFifoWrite((fat_date >> 8) & 0xFF);
    txFifoWrite((fat_time >> 0) & 0xFF);
    txFifoWrite((fat_time >> 8) & 0xFF);
    txFifoWrite((st.st_mode & S_IFDIR) != 0 ? DE_ATTR_DIR : 0);
    txFifoWrite((st.st_size >> 0) & 0xFF);
    txFifoWrite((st.st_size >> 8) & 0xFF);
    txFifoWrite((st.st_size >> 16) & 0xFF);
    txFifoWrite((st.st_size >> 24) & 0xFF);
}

void UartProtocol::cmdGetCwd() {
    DBGF("GETCWD\n");

    txFifoWrite(0);
    int len = (int)currentPath.size();

    txFifoWrite('/');
    for (int i = 0; i < len; i++) {
        txFifoWrite(currentPath[i]);
    }
    txFifoWrite(0);
}

void UartProtocol::cmdCloseAll() {
    DBGF("CLOSEALL\n");
    closeAllDescriptors();
    txFifoWrite(0);

    fi.clear();
    di.clear();
}

void UartProtocol::cmdLoadFpga(const char *pathArg) {
    DBGF("LOADFPGA\n");

    VFS *vfs  = nullptr;
    auto path = resolvePath(pathArg, &vfs);
    if (!vfs) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    struct stat st;
    int         result = vfs->stat(path, &st);
    if (result < 0) {
        txFifoWrite(result);
        return;
    }
    if ((st.st_mode & S_IFREG) == 0) {
        txFifoWrite(ERR_PARAM);
        return;
    }

    void *buf = malloc(st.st_size);
    if (buf == nullptr) {
        printf("Insufficient memory to allocate buffer for bitstream!\n");
        txFifoWrite(ERR_OTHER);
        return;
    }

    int vfs_fd = vfs->open(FO_RDONLY, path);
    if (vfs_fd < 0) {
        txFifoWrite(vfs_fd);
    } else {
        vfs->read(vfs_fd, st.st_size, buf);
        vfs->close(vfs_fd);
        txFifoWrite(0);

        printf("Loading bitstream: %s (%u bytes)\n", pathArg, (unsigned)st.st_size);
    }

    free(buf);
}

void UartProtocol::gameCtrlUpdated() {
    // Update hand controller
    uint8_t handCtrl = 0xFF;

    if (gameCtrlPresent) {
        if (gameCtrlButtons & GCB_A)
            handCtrl &= ~(1 << 6);
        if (gameCtrlButtons & GCB_B)
            handCtrl &= ~((1 << 7) | (1 << 2));
        if (gameCtrlButtons & GCB_X)
            handCtrl &= ~((1 << 7) | (1 << 5));
        if (gameCtrlButtons & GCB_Y)
            handCtrl &= ~((1 << 5));
        if (gameCtrlButtons & GCB_LB)
            handCtrl &= ~((1 << 7) | (1 << 1));
        if (gameCtrlButtons & GCB_RB)
            handCtrl &= ~((1 << 7) | (1 << 0));

        // Map D-pad on hand controller disc
        unsigned p = 0;
        if ((gameCtrlButtons & GCB_DPAD_UP) == GCB_DPAD_UP)
            p = 13;
        else if ((gameCtrlButtons & (GCB_DPAD_UP | GCB_DPAD_RIGHT)) == (GCB_DPAD_UP | GCB_DPAD_RIGHT))
            p = 15;
        else if ((gameCtrlButtons & GCB_DPAD_RIGHT) == GCB_DPAD_RIGHT)
            p = 1;
        else if ((gameCtrlButtons & (GCB_DPAD_DOWN | GCB_DPAD_RIGHT)) == (GCB_DPAD_DOWN | GCB_DPAD_RIGHT))
            p = 3;
        else if ((gameCtrlButtons & GCB_DPAD_DOWN) == GCB_DPAD_DOWN)
            p = 5;
        else if ((gameCtrlButtons & (GCB_DPAD_DOWN | GCB_DPAD_LEFT)) == (GCB_DPAD_DOWN | GCB_DPAD_LEFT))
            p = 7;
        else if ((gameCtrlButtons & GCB_DPAD_LEFT) == GCB_DPAD_LEFT)
            p = 9;
        else if ((gameCtrlButtons & (GCB_DPAD_UP | GCB_DPAD_LEFT)) == (GCB_DPAD_UP | GCB_DPAD_LEFT))
            p = 11;

        {
            float x = gameCtrlLX / 128.0f;
            float y = gameCtrlLY / 128.0f;

            float len   = sqrtf(x * x + y * y);
            float angle = 0;
            if (len > 0.4f) {
                angle = atan2f(y, x) / (float)M_PI * 180.0f + 180.0f;
                p     = ((int)((angle + 11.25) / 22.5f) + 8) % 16 + 1;
            }
        }

        switch (p) {
            case 1: handCtrl &= ~((1 << 1)); break;
            case 2: handCtrl &= ~((1 << 4) | (1 << 1)); break;
            case 3: handCtrl &= ~((1 << 4) | (1 << 1) | (1 << 0)); break;
            case 4: handCtrl &= ~((1 << 1) | (1 << 0)); break;
            case 5: handCtrl &= ~((1 << 0)); break;
            case 6: handCtrl &= ~((1 << 4) | (1 << 0)); break;
            case 7: handCtrl &= ~((1 << 4) | (1 << 3) | (1 << 0)); break;
            case 8: handCtrl &= ~((1 << 3) | (1 << 0)); break;
            case 9: handCtrl &= ~((1 << 3)); break;
            case 10: handCtrl &= ~((1 << 4) | (1 << 3)); break;
            case 11: handCtrl &= ~((1 << 4) | (1 << 3) | (1 << 2)); break;
            case 12: handCtrl &= ~((1 << 3) | (1 << 2)); break;
            case 13: handCtrl &= ~((1 << 2)); break;
            case 14: handCtrl &= ~((1 << 4) | (1 << 2)); break;
            case 15: handCtrl &= ~((1 << 4) | (1 << 2) | (1 << 1)); break;
            case 16: handCtrl &= ~((1 << 2) | (1 << 1)); break;
            default: break;
        }
    }

    auto aqk               = Keyboard::instance();
    aqk->handCtrl_gameCtrl = handCtrl;
    aqk->updateMatrix();
}
