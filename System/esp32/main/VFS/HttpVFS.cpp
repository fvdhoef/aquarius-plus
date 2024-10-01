#include "HttpVFS.h"
#include <esp_http_client.h>

#define MAX_FDS (10)

struct state {
    esp_http_client_handle_t clients[MAX_FDS];
};

static struct state state;

HttpVFS::HttpVFS() {
}

HttpVFS &HttpVFS::instance() {
    static HttpVFS vfs;
    return vfs;
}

void HttpVFS::init() {
}

int HttpVFS::open(uint8_t flags, const std::string &_path) {
    (void)flags;
    printf("HTTP open: %s\n", _path.c_str());

    if (flags == FO_RDONLY) {
        // Find free file descriptor
        int fd = -1;
        for (int i = 0; i < MAX_FDS; i++) {
            if (state.clients[i] == nullptr) {
                fd = i;
                break;
            }
        }
        if (fd == -1)
            return ERR_TOO_MANY_OPEN;

        esp_http_client_config_t config = {
            .url = _path.c_str(),
        };
        state.clients[fd] = esp_http_client_init(&config);

        auto client = state.clients[fd];

        esp_err_t err = esp_http_client_open(client, 0);
        if (err != ESP_OK) {
            close(fd);
            return ERR_OTHER;
        }

        int content_length = esp_http_client_fetch_headers(client);
        if (content_length < 0) {
            close(fd);
            return ERR_OTHER;
        }

        int status_code = esp_http_client_get_status_code(state.clients[fd]);
        if (status_code != 200) {
            close(fd);
            if (status_code == 404)
                return ERR_NOT_FOUND;

            return ERR_OTHER;
        }
        return fd;
    }
    return ERR_OTHER;
}

int HttpVFS::read(int fd, size_t size, void *buf) {
    (void)buf;
    printf("HTTP read: %d  size: %u\n", fd, (unsigned)size);
    if (fd >= MAX_FDS || state.clients[fd] == nullptr)
        return ERR_PARAM;
    auto client = state.clients[fd];

    int result = esp_http_client_read(client, (char *)buf, size);
    if (result >= 0)
        return result;

    return ERR_OTHER;
}

int HttpVFS::write(int fd, size_t size, const void *buf) {
    (void)buf;
    printf("HTTP write: %d  size: %u\n", fd, (unsigned)size);
    if (fd >= MAX_FDS || state.clients[fd] == nullptr)
        return ERR_PARAM;
    return ERR_OTHER;
}

int HttpVFS::close(int fd) {
    printf("HTTP close: %d\n", fd);
    if (fd >= MAX_FDS || state.clients[fd] == nullptr)
        return ERR_PARAM;
    auto client = state.clients[fd];
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    state.clients[fd] = nullptr;
    return 0;
}
