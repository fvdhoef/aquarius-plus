#include "VFS.h"
#ifdef _WIN32
#include <Windows.h>
#include <wininet.h>
#pragma comment(lib, "Wininet.lib")
#endif

#define MAX_FDS (10)

class HttpVFS : public VFS {
public:
#ifdef _WIN32
    static HINTERNET hInternet;
    HINTERNET        clients[MAX_FDS];
#endif

    HttpVFS() {
    }

    void init() override {
#ifdef _WIN32
        hInternet = InternetOpenA("Aquarius+ emulator", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
#endif
    }

    int open(uint8_t flags, const std::string &_path) override {
        (void)flags;
        printf("HTTP open: %s\n", _path.c_str());

#ifdef _WIN32
        if (flags == FO_RDONLY) {
            // Find free file descriptor
            int fd = -1;
            for (int i = 0; i < MAX_FDS; i++) {
                if (clients[i] == nullptr) {
                    fd = i;
                    break;
                }
            }
            if (fd == -1)
                return ERR_TOO_MANY_OPEN;

            clients[fd] = InternetOpenUrlA(hInternet, _path.c_str(), NULL, 0, INTERNET_FLAG_NO_UI | INTERNET_FLAG_RELOAD, NULL);
            if (clients[fd] == NULL)
                return ERR_OTHER;

            {
                char responseText[256];
                responseText[0]        = 0;
                DWORD responseTextSize = sizeof(responseText);

                // Check existance of page (for 404 error)
                if (!HttpQueryInfoA(clients[fd], HTTP_QUERY_STATUS_CODE, responseText, &responseTextSize, NULL)) {
                    close(fd);
                    return ERR_OTHER;
                }
                int status_code = atoi(responseText);
                if (status_code != 200) {
                    close(fd);
                    if (status_code == 404)
                        return ERR_NOT_FOUND;

                    return ERR_OTHER;
                }
            }
            return fd;
        }
#endif
        return ERR_OTHER;
    }

    int read(int fd, size_t size, void *buf) override {
        (void)buf;
        printf("HTTP read: %d  size: %u\n", fd, (unsigned)size);
#ifdef _WIN32
        if (fd >= MAX_FDS || clients[fd] == nullptr)
            return ERR_PARAM;
        auto client = clients[fd];

        DWORD bytes_read = 0;
        if (InternetReadFile(client, buf, (DWORD)size, &bytes_read)) {
            return bytes_read;
        }

#endif
        return ERR_OTHER;
    }

    int write(int fd, size_t size, const void *buf) override {
        (void)buf;
        printf("HTTP write: %d  size: %u\n", fd, (unsigned)size);
#ifdef _WIN32
        if (fd >= MAX_FDS || clients[fd] == nullptr)
            return ERR_PARAM;
#endif
        return ERR_OTHER;
    }

    int close(int fd) override {
        printf("HTTP close: %d\n", fd);
#ifdef _WIN32
        if (fd >= MAX_FDS || clients[fd] == nullptr)
            return ERR_PARAM;
        auto client = clients[fd];
        InternetCloseHandle(client);
        clients[fd] = nullptr;
#endif
        return 0;
    }
};

VFS *getHttpVFS() {
    static HttpVFS obj;
    return &obj;
}
