#include "fileserver.h"
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "SDCardVFS.h"
#include <errno.h>
#include <mdns.h>
#include "aq_keyb.h"

// #include <esp_vfs.h>

static const char *TAG = "fileserver";

#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

static char *urlencode(const char *s) {
    size_t len    = strlen(s);
    char  *result = (char *)malloc(3 * len + 1);
    if (result == nullptr) {
        return nullptr;
    }

    const char  hexlut[] = "0123456789ABCDEF";
    const char *ps       = s;
    char       *pd       = result;

    while (*ps != '\0') {
        char ch = *(ps++);

        if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
            (ch == '_' || ch == '-' || ch == '.' || ch == '/')) {
            *(pd++) = ch;
        } else if (ch == ' ') {
            *(pd++) = '+';
        } else {
            *(pd++) = '%';
            *(pd++) = hexlut[ch >> 4];
            *(pd++) = hexlut[ch & 15];
        }
    }
    *(pd++) = 0;
    return result;
}

static void urldecode(char *s) {
    char *ps = s;
    char *pd = s;

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
                    goto done;

                ch = (ch << 4) | tmp;
            }
        }
        *(pd++) = ch;
    }
done:
    *(pd++) = 0;
}

static const char *get_path_from_uri(char *dest, size_t destsize, const char *uri) {
    size_t pathlen = strlen(uri);

    const char *quest = strchr(uri, '?');
    if (quest) {
        pathlen = MIN(pathlen, quest - uri);
    }
    const char *hash = strchr(uri, '#');
    if (hash) {
        pathlen = MIN(pathlen, hash - uri);
    }

    if (pathlen + 1 > destsize) {
        /* Full path string won't fit into destination buffer */
        return nullptr;
    }

    /* Construct full path (base + path) */
    strlcpy(dest, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */

    urldecode(dest);

    return dest;
}

static esp_err_t handler_options(httpd_req_t *req) {
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
    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, nullptr, 0);
    return ESP_OK;
}

static void propfind_st(httpd_req_t *req, const char *href_tag, struct stat *st) {
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

static esp_err_t handler_propfind(httpd_req_t *req) {
    char  depth[16] = "0";
    int   size      = 0;
    int   result    = 0;
    char *encoded   = nullptr;

    // Allocate buffers
    const size_t path_size = 512;
    const size_t tmp_size  = 1024;
    char        *uripath   = (char *)malloc(path_size);
    char        *path      = (char *)malloc(path_size);
    char        *tmp       = (char *)malloc(tmp_size);

    if (!uripath || !path || !tmp)
        goto error;

    // Get hostname in request header
    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        goto error;

    httpd_req_get_hdr_value_str(req, "Depth", depth, sizeof(depth));

    // Get payload
    size = httpd_req_recv(req, tmp, tmp_size - 1);
    if (size > 0) {
        tmp[size] = '\0';
    }

    // Get path
    get_path_from_uri(uripath, path_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, uripath);

    // ESP_LOGI(TAG, "PROPFIND: %s", path);

    httpd_resp_set_hdr(req, "DAV", "1");

    // Check file
    struct stat st;
    result = stat(path, &st);
    if (result < 0) {
        httpd_resp_send_404(req);
        goto done;
    }

    httpd_resp_set_type(req, "text/xml; encoding=\"utf-8\"");
    httpd_resp_set_status(req, HTTPD_207);
    httpd_resp_sendstr_chunk(req, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
    httpd_resp_sendstr_chunk(req, "<multistatus xmlns=\"DAV:\">");

    encoded = urlencode(uripath);
    snprintf(tmp, tmp_size, "<href>http://%s%s</href>", host, encoded);
    free(encoded);

    propfind_st(req, tmp, &st);

    if (S_ISDIR(st.st_mode) && depth[0] != '0') {
        DIR *dir = opendir(path);
        if (dir != nullptr) {
            // printf("Host: %s\n", host);
            while (1) {
                struct dirent *de = readdir(dir);
                if (de == nullptr)
                    break;

                snprintf(tmp, tmp_size, "%s/%s", path, de->d_name);
                int result = stat(tmp, &st);
                if (result < 0) {
                    continue;
                }

                char *encoded = urlencode(de->d_name);
                snprintf(tmp, tmp_size, "<href>http://%s%s%s</href>", host, uripath, encoded);
                free(encoded);

                propfind_st(req, tmp, &st);
            }
            closedir(dir);
        }
    }
    httpd_resp_sendstr_chunk(req, "</multistatus>");
    httpd_resp_sendstr_chunk(req, nullptr);

done:
    free(uripath);
    free(path);
    free(tmp);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, size, ESP_LOG_INFO);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_delete(httpd_req_t *req) {
    // Allocate buffers
    const size_t path_size = 512;
    char        *uripath   = (char *)malloc(path_size);
    char        *path      = (char *)malloc(path_size);
    if (!uripath || !path)
        goto error;

    // Get path
    get_path_from_uri(uripath, path_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, uripath);

    // Unlink file
    if (unlink(path) == 0) {
        httpd_resp_set_status(req, HTTPD_204);
        httpd_resp_send(req, nullptr, 0);
    } else if (errno == ENOENT) {
        httpd_resp_send_404(req);
    } else {
        goto error;
    }

done:
    free(uripath);
    free(path);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_get(httpd_req_t *req) {
    int result = 0;

    // Allocate buffers
    const size_t path_size = 512;
    const size_t tmp_size  = 16384;
    char        *uripath   = (char *)malloc(path_size);
    char        *path      = (char *)malloc(path_size);
    char        *tmp       = (char *)malloc(tmp_size);
    if (!uripath || !path || !tmp)
        goto error;

    // Get path
    get_path_from_uri(tmp, tmp_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);

    struct stat st;
    result = stat(path, &st);
    if (result < 0) {
        httpd_resp_send_404(req);
        goto done;
    }

    if (S_ISDIR(st.st_mode)) {
        httpd_resp_set_type(req, "text/html; charset=utf-8");

        httpd_resp_sendstr_chunk(req, "<!DOCTYPE html><html><body>");
        httpd_resp_sendstr_chunk(
            req,
            "<table class=\"fixed\" border=\"1\">"
            "<thead><tr><th>Name</th><th align=\"right\">Size (Bytes)</th></tr></thead>"
            "<tbody>");

        DIR *dir = opendir(path);
        if (dir != nullptr) {
            // printf("Host: %s\n", host);
            while (1) {
                struct dirent *de = readdir(dir);
                if (de == nullptr)
                    break;

                snprintf(tmp, tmp_size, "%s/%s", path, de->d_name);
                int result = stat(tmp, &st);
                if (result < 0) {
                    continue;
                }

                char *encoded = urlencode(de->d_name);
                snprintf(tmp, tmp_size, "<tr><td><a href=\"%s%s\">%s</href></td>", req->uri, encoded, de->d_name);
                free(encoded);
                httpd_resp_sendstr_chunk(req, tmp);

                if (S_ISDIR(st.st_mode)) {
                    snprintf(tmp, tmp_size, "<td align=\"right\">Directory</td></tr>");
                } else {
                    snprintf(tmp, tmp_size, "<td align=\"right\">%lu</td></tr>", st.st_size);
                }
                httpd_resp_sendstr_chunk(req, tmp);
            }
            closedir(dir);
        }

        httpd_resp_sendstr_chunk(req, "</tbody></table>");
        httpd_resp_sendstr_chunk(req, "</body></html>");
        httpd_resp_sendstr_chunk(req, nullptr);

    } else {
        FILE *f = fopen(path, "rb");
        if (!f) {
            httpd_resp_send_404(req);
        } else {
            httpd_resp_set_type(req, "application/octet-stream");
            while (1) {
                size_t size = fread(tmp, 1, tmp_size, f);
                httpd_resp_send_chunk(req, (const char *)tmp, size);
                if (size == 0) {
                    break;
                }
            }
            fclose(f);
        }
    }

done:
    free(uripath);
    free(path);
    free(tmp);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_put(httpd_req_t *req) {
    int   remaining = req->content_len;
    FILE *f         = nullptr;

    // Allocate buffers
    const size_t path_size = 512;
    const size_t tmp_size  = 16384;
    char        *path      = (char *)malloc(path_size);
    char        *tmp       = (char *)malloc(tmp_size);
    if (!path || !tmp)
        goto error;

    // Get path
    get_path_from_uri(tmp, tmp_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);

    // printf("PUT %s\n", path);

    if ((f = fopen(path, "wb")) == nullptr)
        goto error;

    while (remaining > 0) {
        // ESP_LOGI(TAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer
        int received;
        if ((received = httpd_req_recv(req, tmp, MIN(remaining, tmp_size))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred
                continue;
            }

            // In case of unrecoverable error, close and delete the unfinished file
            fclose(f);
            unlink(path);
            goto error;
        }

        // Write buffer content to file on storage
        if (received && (received != fwrite(tmp, 1, received, f))) {
            // Couldn't write everything to file! Storage may be full?
            fclose(f);
            unlink(path);
            goto error;
        }

        // Keep track of remaining size of the file left to be uploaded
        remaining -= received;
    }
    fclose(f);

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, nullptr, 0);

done:
    free(path);
    free(tmp);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_move(httpd_req_t *req) {
    char *p = nullptr;

    // Allocate buffers
    const size_t path_size = 512;
    const size_t tmp_size  = 1024;
    char        *path      = (char *)malloc(path_size);
    char        *path2     = (char *)malloc(path_size);
    char        *tmp       = (char *)malloc(tmp_size);
    if (!path || !path2 || !tmp)
        goto error;

    // Get path
    get_path_from_uri(tmp, tmp_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);

    // Get destination path from request header
    if (httpd_req_get_hdr_value_str(req, "Destination", tmp, tmp_size) != ESP_OK)
        goto error;

    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        goto error;

    p = strcasestr(tmp, host);
    if (p == nullptr || p - tmp > 8) {
        httpd_resp_send_404(req);
        goto done;
    }
    p += strlen(host);
    urldecode(p);

    snprintf(path2, path_size, "%s%s", MOUNT_POINT, p);
    // printf("MOVE %s to %s\n", path, path2);
    if (rename(path, path2) == 0) {
        httpd_resp_set_status(req, HTTPD_204);
        httpd_resp_send(req, nullptr, 0);
    } else {
        httpd_resp_send_404(req);
    }

done:
    free(path);
    free(path2);
    free(tmp);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_mkcol(httpd_req_t *req) {
    // Allocate buffers
    const size_t path_size = 512;
    char        *uripath   = (char *)malloc(path_size);
    char        *path      = (char *)malloc(path_size);
    if (!uripath || !path)
        goto error;

    // Get path
    get_path_from_uri(uripath, path_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, uripath);

    // Unlink file
    if (mkdir(path, 0775) == 0) {
        httpd_resp_set_status(req, HTTPD_204);
        httpd_resp_send(req, nullptr, 0);
    } else {
        goto error;
    }

done:
    free(uripath);
    free(path);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_copy(httpd_req_t *req) {
    FILE *f  = nullptr;
    FILE *f2 = nullptr;
    char *p  = nullptr;

    // Allocate buffers
    const size_t path_size = 512;
    const size_t tmp_size  = 16384;
    char        *path      = (char *)malloc(path_size);
    char        *path2     = (char *)malloc(path_size);
    char        *tmp       = (char *)malloc(tmp_size);
    if (!path || !path2 || !tmp)
        goto error;

    // Get path
    get_path_from_uri(tmp, tmp_size, req->uri);
    snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);

    // Get destination path from request header
    if (httpd_req_get_hdr_value_str(req, "Destination", tmp, tmp_size) != ESP_OK)
        goto error;

    char host[64];
    if (httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host)) != ESP_OK)
        goto error;

    p = strcasestr(tmp, host);
    if (p == nullptr || p - tmp > 8) {
        httpd_resp_send_404(req);
        goto done;
    }
    p += strlen(host);
    urldecode(p);

    snprintf(path2, path_size, "%s%s", MOUNT_POINT, p);
    // printf("COPY %s to %s\n", path, path2);

    if ((f = fopen(path, "rb")) == nullptr) {
        httpd_resp_send_404(req);
        goto done;
    }
    if ((f2 = fopen(path2, "wb")) == nullptr) {
        goto error;
    }

    while (1) {
        size_t size = fread(tmp, 1, tmp_size, f);
        if (size == 0)
            break;

        if (fwrite(tmp, 1, size, f2) != size) {
            fclose(f2);
            f2 = nullptr;
            unlink(path2);
            goto error;
        }
    }

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, nullptr, 0);

done:
    if (f != nullptr)
        fclose(f);
    if (f2 != nullptr)
        fclose(f2);

    free(path);
    free(path2);
    free(tmp);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

static esp_err_t handler_post_keyboard(httpd_req_t *req) {
    int remaining = req->content_len;

    // Allocate buffers
    const size_t tmp_size = 16384;
    char        *tmp      = (char *)malloc(tmp_size);
    if (!tmp)
        goto error;

    while (remaining > 0) {
        // ESP_LOGI(TAG, "Remaining size : %d", remaining);
        // Receive the file part by part into a buffer
        int received;
        if ((received = httpd_req_recv(req, tmp, MIN(remaining, tmp_size))) <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                // Retry if timeout occurred
                continue;
            }
            goto error;
        }

        // Write buffer to keyboard
        for (int i = 0; i < received; i++) {
            keyboard_press_key(tmp[i]);
        }

        // Keep track of remaining size of the file left to be uploaded
        remaining -= received;
    }

    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, nullptr, 0);

done:
    free(tmp);
    return ESP_OK;

error:
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    goto done;
}

void fileserver_init(void) {
    esp_err_t err = mdns_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "MDNS Init failed: %d\n", err);
    } else {
        mdns_hostname_set(CONFIG_LWIP_LOCAL_HOSTNAME);
        mdns_instance_name_set("Aquarius+");
    }

    httpd_handle_t server   = nullptr;
    httpd_config_t config   = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 10;
    config.uri_match_fn     = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return;
    }

    {
        httpd_uri_t hu = {.uri = "/keyboard", .method = HTTP_POST, .handler = handler_post_keyboard};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_DELETE, .handler = handler_delete};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_GET, .handler = handler_get};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_PUT, .handler = handler_put};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_COPY, .handler = handler_copy};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_MKCOL, .handler = handler_mkcol};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_MOVE, .handler = handler_move};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_OPTIONS, .handler = handler_options};
        httpd_register_uri_handler(server, &hu);
    }
    {
        httpd_uri_t hu = {.uri = "/*", .method = HTTP_PROPFIND, .handler = handler_propfind};
        httpd_register_uri_handler(server, &hu);
    }
}
