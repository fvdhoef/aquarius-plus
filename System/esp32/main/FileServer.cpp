#include "FileServer.h"
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "SDCardVFS.h"
#include <errno.h>
#include <mdns.h>
#include "aq_keyb.h"

// #include <esp_vfs.h>

static const char *TAG = "FileServer";

static esp_err_t serverError(httpd_req_t *req) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    return ESP_OK;
}

static esp_err_t resp404(httpd_req_t *req) {
    httpd_resp_send_404(req);
    return ESP_OK;
}

static esp_err_t resp204(httpd_req_t *req) {
    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

FileServer::FileServer() {
    server = nullptr;
}

FileServer &FileServer::instance() {
    static FileServer obj;
    return obj;
}

void FileServer::init() {
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS Init failed: %d\n", err);
    } else {
        mdns_hostname_set(CONFIG_LWIP_LOCAL_HOSTNAME);
        mdns_instance_name_set("Aquarius+");
    }

    httpd_config_t config   = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    config.uri_match_fn     = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return;
    }

    registerHandler("/keyboard", HTTP_POST, [&](httpd_req_t *req) { return postKeyboard(req); });
    registerHandler("/*", HTTP_DELETE, [&](httpd_req_t *req) { return handleDelete(req); });
    registerHandler("/*", HTTP_GET, [&](httpd_req_t *req) { return handleGet(req); });
    registerHandler("/*", HTTP_PUT, [&](httpd_req_t *req) { return handlePut(req); });
    registerHandler("/*", HTTP_COPY, [&](httpd_req_t *req) { return handleCopy(req); });
    registerHandler("/*", HTTP_MKCOL, [&](httpd_req_t *req) { return handleMkCol(req); });
    registerHandler("/*", HTTP_MOVE, [&](httpd_req_t *req) { return handleMove(req); });
    registerHandler("/*", HTTP_OPTIONS, [&](httpd_req_t *req) { return handleOptions(req); });
    registerHandler("/*", HTTP_PROPFIND, [&](httpd_req_t *req) { return handlePropFind(req); });
}

esp_err_t FileServer::_handler(httpd_req_t *req) {
    auto ctx = static_cast<HandlerContext *>(req->user_ctx);
    return ctx->handler(req);
}

void FileServer::registerHandler(const std::string &uri, httpd_method_t method, HttpHandler handler) {
    auto ctx     = new HandlerContext;
    ctx->handler = handler;

    httpd_uri_t hu = {.uri = uri.c_str(), .method = method, .handler = _handler, .user_ctx = ctx};
    httpd_register_uri_handler(server, &hu);
}

static std::string urlEncode(const std::string &s) {
    std::string result;
    const char  hexlut[] = "0123456789ABCDEF";

    for (auto ch : s) {
        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
            (ch == '_' || ch == '-' || ch == '.' || ch == '/')) {
            result.push_back(ch);
        } else if (ch == ' ') {
            result.push_back('+');
        } else {
            result.push_back('%');
            result.push_back(hexlut[ch >> 4]);
            result.push_back(hexlut[ch & 15]);
        }
    }
    return result;
}

static std::string urlDecode(const std::string &s) {
    std::string result;
    const char *ps = s.c_str();

    while (*ps != '\0') {
        char ch = *(ps++);

        if (ch == '%') {
            ch = 0;
            for (int i = 0; i < 2; i++) {
                uint8_t tmp = *(ps++);
                if (tmp >= '0' && tmp <= '9')
                    tmp -= '0';
                else if (tmp >= 'a' && tmp <= 'z')
                    tmp = tmp - 'a' + 10;
                else if (tmp >= 'A' && tmp <= 'Z')
                    tmp = tmp - 'A' + 10;
                else
                    break;

                ch = (ch << 4) | tmp;
            }
        }
        result.push_back(ch);
    }
    return result;
}

static std::string getPathFromUri(const std::string &uri) {
    return urlDecode(uri.substr(0, std::min(uri.size(), uri.find_first_of("?#"))));
}

esp_err_t FileServer::handleOptions(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "DAV", "1");
    httpd_resp_set_hdr(
        req, "Allow",
        "OPTIONS, "  // Done
        "PROPFIND, " // Done
        // "PROPPATCH, "
        "MKCOL, " // Done
        "GET, "   // Done
        // "HEAD, "
        "POST, "
        "DELETE, " // Done
        "PUT, "    // Done
        "COPY, "   // Done
        "MOVE"     // Done
    );
    return resp204(req);
}

static void propfind_st(httpd_req_t *req, const char *href_tag, const struct stat *st) {
    char tmp[128];

    httpd_resp_sendstr_chunk(req, "<response>");
    httpd_resp_sendstr_chunk(req, href_tag);
    httpd_resp_sendstr_chunk(req, "<propstat><prop>");

    if (S_ISDIR(st->st_mode)) {
        httpd_resp_sendstr_chunk(req, "<resourcetype><collection/></resourcetype>");
    } else {
        httpd_resp_sendstr_chunk(req, "<resourcetype/>");

        snprintf(tmp, sizeof(tmp), "<getcontentlength>%lu</getcontentlength>", st->st_size);
        httpd_resp_sendstr_chunk(req, tmp);
    }

    struct tm tm;
    localtime_r(&st->st_mtim.tv_sec, &tm);
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%a, %d %b %Y %T %z", &tm);
    snprintf(tmp, sizeof(tmp), "<getlastmodified>%s</getlastmodified>", strftime_buf);
    httpd_resp_sendstr_chunk(req, tmp);

    httpd_resp_sendstr_chunk(req, "</prop><status>HTTP/1.1 200 OK</status></propstat></response>");
}

esp_err_t FileServer::handlePropFind(httpd_req_t *req) {
    // Get hostname in request header
    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        return ESP_FAIL;

    // Get depth
    char depth[16] = "0";
    httpd_req_get_hdr_value_str(req, "Depth", depth, sizeof(depth));

    // Get payload
    const size_t tmpSize = 1024;
    auto         tmp     = std::make_unique<char[]>(tmpSize);
    int          size    = httpd_req_recv(req, tmp.get(), tmpSize - 1);
    if (size > 0) {
        tmp.get()[size] = '\0';
    }

    // Get path
    auto uriPath = getPathFromUri(req->uri);
    auto path    = MOUNT_POINT + uriPath;

    // ESP_LOGI(TAG, "PROPFIND: %s", path);

    httpd_resp_set_hdr(req, "DAV", "1");

    // Check file
    struct stat st;
    if (stat(path.c_str(), &st) < 0)
        return resp404(req);

    httpd_resp_set_type(req, "text/xml; encoding=\"utf-8\"");
    httpd_resp_set_status(req, HTTPD_207);
    httpd_resp_sendstr_chunk(req, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    httpd_resp_sendstr_chunk(req, "<multistatus xmlns=\"DAV:\">");

    snprintf(tmp.get(), tmpSize, "<href>http://%s%s</href>", host, urlEncode(uriPath).c_str());
    propfind_st(req, tmp.get(), &st);

    if (S_ISDIR(st.st_mode) && depth[0] != '0') {
        DIR *dir = opendir(path.c_str());
        if (dir != nullptr) {
            // printf("Host: %s\n", host);
            while (1) {
                struct dirent *de = readdir(dir);
                if (de == nullptr)
                    break;

                snprintf(tmp.get(), tmpSize, "%s/%s", path.c_str(), de->d_name);
                if (stat(tmp.get(), &st) < 0) {
                    continue;
                }

                snprintf(tmp.get(), tmpSize, "<href>http://%s%s%s</href>", host, uriPath.c_str(), urlEncode(de->d_name).c_str());
                propfind_st(req, tmp.get(), &st);
            }
            closedir(dir);
        }
    }
    httpd_resp_sendstr_chunk(req, "</multistatus>");
    httpd_resp_sendstr_chunk(req, nullptr);
    return ESP_OK;
}

esp_err_t FileServer::handleDelete(httpd_req_t *req) {
    // Get path
    auto path = MOUNT_POINT + getPathFromUri(req->uri);

    // Unlink file
    if (unlink(path.c_str()) != 0) {
        if (errno == ENOENT)
            return resp404(req);
        else
            return serverError(req);
    }
    return resp204(req);
}

esp_err_t FileServer::handleGet(httpd_req_t *req) {
    int result = 0;

    // Allocate buffers
    const size_t tmpSize = 16384;
    auto         tmp     = std::make_unique<char[]>(tmpSize);

    // Get path
    auto uriPath = getPathFromUri(req->uri);
    auto path    = MOUNT_POINT + uriPath;

    struct stat st;
    if ((result = stat(path.c_str(), &st)) < 0)
        return resp404(req);

    if (S_ISDIR(st.st_mode)) {
        httpd_resp_set_type(req, "text/html; charset=utf-8");

        httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");
        httpd_resp_sendstr_chunk(
            req,
            "<table class=\"fixed\" border=\"1\">"
            "<thead><tr><th>Name</th><th align=\"right\">Size (Bytes)</th></tr></thead>"
            "<tbody>");

        DIR *dir = opendir(path.c_str());
        if (dir != nullptr) {
            // printf("Host: %s\n", host);
            while (1) {
                struct dirent *de = readdir(dir);
                if (de == nullptr)
                    break;

                snprintf(tmp.get(), tmpSize, "%s/%s", path.c_str(), de->d_name);
                int result = stat(tmp.get(), &st);
                if (result < 0) {
                    continue;
                }

                snprintf(tmp.get(), tmpSize, "<tr><td><a href=\"%s%s\">%s</href></td>", req->uri, urlEncode(de->d_name).c_str(), de->d_name);
                httpd_resp_sendstr_chunk(req, tmp.get());

                if (S_ISDIR(st.st_mode)) {
                    snprintf(tmp.get(), tmpSize, "<td align=\"right\">Directory</td></tr>");
                } else {
                    snprintf(tmp.get(), tmpSize, "<td align=\"right\">%lu</td></tr>", st.st_size);
                }
                httpd_resp_sendstr_chunk(req, tmp.get());
            }
            closedir(dir);
        }

        httpd_resp_sendstr_chunk(req, "</tbody></table>");
        httpd_resp_sendstr_chunk(req, "</body></html>");
        httpd_resp_sendstr_chunk(req, nullptr);

    } else {
        FILE *f = fopen(path.c_str(), "rb");
        if (!f) {
            return resp404(req);
        } else {
            httpd_resp_set_type(req, "application/octet-stream");
            while (1) {
                size_t size = fread(tmp.get(), 1, tmpSize, f);
                httpd_resp_send_chunk(req, tmp.get(), size);
                if (size == 0) {
                    break;
                }
            }
            fclose(f);
        }
    }
    return ESP_OK;
}

esp_err_t FileServer::handlePut(httpd_req_t *req) {
    // Allocate buffers
    const size_t tmpSize = 16384;
    auto         tmp     = std::make_unique<char[]>(tmpSize);

    // Get path
    auto path = MOUNT_POINT + getPathFromUri(req->uri);

    // printf("PUT %s\n", path);

    // Open target file
    FILE *f = nullptr;
    if ((f = fopen(path.c_str(), "wb")) == nullptr) {
        return serverError(req);
    }

    auto remaining = req->content_len;
    while (remaining > 0) {
        // ESP_LOGI(TAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer
        int received;
        if ((received = httpd_req_recv(req, tmp.get(), std::min(remaining, tmpSize))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred
                continue;
            }

            // In case of unrecoverable error, close and delete the unfinished file
            fclose(f);
            unlink(path.c_str());
            return serverError(req);
        }

        // Write buffer content to file on storage
        if (received && (received != fwrite(tmp.get(), 1, received, f))) {
            // Couldn't write everything to file! Storage may be full?
            fclose(f);
            unlink(path.c_str());
            return serverError(req);
        }

        // Keep track of remaining size of the file left to be uploaded
        remaining -= received;
    }
    fclose(f);

    return resp204(req);
}

esp_err_t FileServer::handleMove(httpd_req_t *req) {
    // Allocate buffers
    const size_t tmpSize = 1024;
    auto         tmp     = std::make_unique<char[]>(tmpSize);

    // Get path
    auto path = MOUNT_POINT + getPathFromUri(req->uri);

    // Get destination path from request header
    if (httpd_req_get_hdr_value_str(req, "Destination", tmp.get(), tmpSize) != ESP_OK)
        return serverError(req);

    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        return serverError(req);

    char *p = strcasestr(tmp.get(), host);
    if (p == nullptr || p - tmp.get() > 8)
        return resp404(req);
    p += strlen(host);

    auto path2 = MOUNT_POINT + urlDecode(p);
    // printf("MOVE %s to %s\n", path, path2);

    // Do rename
    if (rename(path.c_str(), path2.c_str()) != 0) {
        return resp404(req);
    }

    return resp204(req);
}

esp_err_t FileServer::handleMkCol(httpd_req_t *req) {
    // Get path
    auto path = MOUNT_POINT + getPathFromUri(req->uri);

    // Unlink file
    if (mkdir(path.c_str(), 0775) != 0) {
        return serverError(req);
    }

    return resp204(req);
}

esp_err_t FileServer::handleCopy(httpd_req_t *req) {
    // Allocate buffers
    const size_t tmpSize = 16384;
    auto         tmp     = std::make_unique<char[]>(tmpSize);

    // Get path
    auto path = MOUNT_POINT + getPathFromUri(req->uri);

    // Get destination path from request header
    if (httpd_req_get_hdr_value_str(req, "Destination", tmp.get(), tmpSize) != ESP_OK)
        return serverError(req);

    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        return serverError(req);

    char *p = strcasestr(tmp.get(), host);
    if (p == nullptr || p - tmp.get() > 8)
        return resp404(req);
    p += strlen(host);

    auto path2 = MOUNT_POINT + urlDecode(p);

    // printf("COPY %s to %s\n", path, path2);
    FILE *f = nullptr;
    if ((f = fopen(path.c_str(), "rb")) == nullptr) {
        return resp404(req);
    }

    FILE *f2 = nullptr;
    if ((f2 = fopen(path2.c_str(), "wb")) == nullptr) {
        fclose(f);
        return resp404(req);
    }

    while (1) {
        size_t size = fread(tmp.get(), 1, tmpSize, f);
        if (size == 0)
            break;

        if (fwrite(tmp.get(), 1, size, f2) != size) {
            fclose(f);
            fclose(f2);
            f2 = nullptr;
            unlink(path2.c_str());
            return serverError(req);
        }
    }
    fclose(f);
    fclose(f2);

    return resp204(req);
}

esp_err_t FileServer::postKeyboard(httpd_req_t *req) {
    // Allocate buffers
    const size_t tmpSize = 16384;
    auto         tmp     = std::make_unique<char[]>(tmpSize);

    auto remaining = req->content_len;
    while (remaining > 0) {
        // ESP_LOGI(TAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer
        int received;
        if ((received = httpd_req_recv(req, tmp.get(), std::min(remaining, tmpSize))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred
                continue;
            }
            return serverError(req);
        }

        // Write buffer to keyboard
        for (unsigned i = 0; i < received; i++) {
            keyboard_press_key(tmp[i]);
        }

        // Keep track of remaining size of the file left to be uploaded
        remaining -= received;
    }
    return resp204(req);
}
