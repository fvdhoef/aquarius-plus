#include "VFS.h"
#ifdef _WIN32
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <netdb.h>
#endif

#define MAX_FDS (10)

static bool startsWith(const std::string &s1, const std::string &s2, bool caseSensitive = false) {
    if (caseSensitive) {
        return (strncasecmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    } else {
        return (strncmp(s1.c_str(), s2.c_str(), s2.size()) == 0);
    }
}

class TcpVFS : public VFS {
public:
    bool inUse[MAX_FDS]   = {0};
    int  sockets[MAX_FDS] = {0};

    TcpVFS() {
    }

    void init() override {
    }

    int open(uint8_t flags, const std::string &_path) override {
        (void)flags;
        printf("TCP open: %s\n", _path.c_str());

        if (!startsWith(_path, "tcp://")) {
            return ERR_PARAM;
        }
        auto colonPos = _path.find(':', 6);
        if (colonPos == std::string::npos) {
            return ERR_PARAM;
        }

        auto host    = _path.substr(6, colonPos - 6);
        auto portStr = _path.substr(colonPos + 1);

        char *endp;
        int   port = strtol(portStr.c_str(), &endp, 10);
        if (endp != portStr.c_str() + portStr.size() || port < 0 || port > 65535) {
            return ERR_PARAM;
        }

        printf("TCP host '%s'  port '%d'\n", host.c_str(), port);

        // Find free file descriptor
        int fd = -1;
        for (int i = 0; i < MAX_FDS; i++) {
            if (!inUse[i]) {
                fd = i;
                break;
            }
        }
        if (fd == -1)
            return ERR_TOO_MANY_OPEN;

        // Resolve name
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_family   = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        struct addrinfo *ai;
        if (getaddrinfo(host.c_str(), portStr.c_str(), &hints, &ai) != 0) {
            return ERR_NOT_FOUND;
        }

        printf("Name resolved!\n");

        // Open socket
        auto sock = socket(ai->ai_family, ai->ai_socktype, ai->ai_protocol);

        if (sock == -1) {
            freeaddrinfo(ai);
            return ERR_OTHER;
        }

        if (fcntl(sock, F_SETFL, fcntl(sock, F_GETFL) | O_NONBLOCK) == -1) {
            ::close(sock);
            return ERR_OTHER;
        }

        printf("Socket opened!\n");

        // Connect to host
        int err = connect(sock, ai->ai_addr, (socklen_t)ai->ai_addrlen);
        freeaddrinfo(ai);

        if (err != 0) {
            if (errno == EINPROGRESS) {
                struct timeval tv;
                tv.tv_sec  = 5;
                tv.tv_usec = 0;

                fd_set fdset;
                FD_ZERO(&fdset);
                FD_SET(sock, &fdset);

                // Connection in progress -> have to wait until the connecting socket is marked as writable, i.e. connection completes
                err = select(sock + 1, NULL, &fdset, NULL, &tv);
                if (err <= 0) {
                    ::close(sock);
                    return ERR_OTHER;
                }

            } else {
                printf("Error connecting to host!\n");
                ::close(sock);
                return ERR_NOT_FOUND;
            }
        }

        // Success!
        inUse[fd]   = true;
        sockets[fd] = sock;
        return fd;
    }

    int read(int fd, size_t size, void *buf) override {
        // printf("TCP read: %d  size: %u\n", fd, (unsigned)size);

        if (fd >= MAX_FDS || !inUse[fd])
            return ERR_PARAM;
        auto sock = sockets[fd];

        if (size == 0)
            return 0;

        int len = recv(sock, buf, size, 0);
        if (len == 0)
            return ERR_EOF;

        if (len < 0) {
            if (errno == EINPROGRESS || errno == EAGAIN || errno == EWOULDBLOCK) {
                return 0; // Not an error
            }
            if (errno == ENOTCONN) {
                return ERR_EOF;
            }
            return ERR_OTHER;
        }
        return len;
    }

    int write(int fd, size_t size, const void *buf) override {
        // printf("TCP write: %d  size: %u\n", fd, (unsigned)size);

        if (fd >= MAX_FDS || !inUse[fd])
            return ERR_PARAM;
        auto sock = sockets[fd];

        if (size == 0)
            return 0;

        int         to_write = (int)size;
        const char *data     = (const char *)buf;
        while (to_write > 0) {
            int written = send(sock, data + (size - to_write), to_write, 0);
            if (written < 0 && errno != EINPROGRESS && errno != EAGAIN && errno != EWOULDBLOCK)
                return ERR_OTHER;

            if (written > 0)
                to_write -= written;
        }
        return (int)size;
    }

    int close(int fd) override {
        printf("TCP close: %d\n", fd);

        if (fd >= MAX_FDS || !inUse[fd])
            return ERR_PARAM;
        ::close(sockets[fd]);
        inUse[fd]   = false;
        sockets[fd] = -1;

        return 0;
    }
};

VFS *getTcpVFS() {
    static TcpVFS obj;
    return &obj;
}
