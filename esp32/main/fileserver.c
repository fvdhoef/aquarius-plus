#include "fileserver.h"
#include <esp_http_server.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include "sdcard.h"
#include <errno.h>

// #include <esp_vfs.h>

static const char *TAG = "fileserver";

#ifndef MIN
#    define MIN(a, b) (((a) < (b)) ? (a) : (b))
#endif

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
        return NULL;
    }

    // Remove trailing slashes
    while (pathlen > 0 && uri[pathlen - 1] == '/') {
        pathlen--;
    }

    /* Construct full path (base + path) */
    strlcpy(dest, uri, pathlen + 1);

    /* Return pointer to path, skipping the base */
    return dest;
}

esp_err_t handler_options(httpd_req_t *req) {
    httpd_resp_set_hdr(req, "DAV", "1");
    httpd_resp_set_hdr(
        req, "Allow",
        "OPTIONS, "  // Done
        "PROPFIND, " // Done
        "PROPPATCH, "
        "MKCOL, "
        "GET, " // Done
        "HEAD, "
        "POST, "
        "DELETE, " // Done
        "PUT, "
        "COPY, "
        "MOVE");
    httpd_resp_set_status(req, HTTPD_204);
    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t handler_propfind(httpd_req_t *req) {
    const size_t path_size = 512;
    const size_t buf_size  = 1024;
    const size_t tmp_size  = 1024;

    char *path = malloc(path_size);
    char *buf  = malloc(buf_size);
    char *tmp  = malloc(tmp_size);

    if (path && buf && tmp) {
        get_path_from_uri(path, 512, req->uri);

        char buf[1024];
        int  size = httpd_req_recv(req, buf, buf_size - 1);
        if (size > 0) {
            buf[size] = '\0';
            ESP_LOGI(TAG, "PROPFIND: %s", buf);
        }

        char host[64];
        httpd_req_get_hdr_value_str(req, "Host", host, sizeof(host));

        snprintf(tmp, tmp_size, "%s%s", MOUNT_POINT, path);
        printf("Opening directory: %s\n", tmp);
        DIR *dir = opendir(tmp);
        if (dir != NULL) {
            // printf("Host: %s\n", host);
            httpd_resp_set_hdr(req, "DAV", "1");
            httpd_resp_set_hdr(req, "Content-Type", "text/xml; encoding=\"utf-8\"");
            httpd_resp_set_status(req, HTTPD_207);
            httpd_resp_sendstr_chunk(req, "<?xml version=\"1.0\" encoding=\"utf-8\"?>");
            httpd_resp_sendstr_chunk(req, "<multistatus xmlns=\"DAV:\">");

            httpd_resp_sendstr_chunk(req, "<response>");
            snprintf(tmp, tmp_size, "<href>http://%s%s</href>", host, path);
            httpd_resp_sendstr_chunk(req, tmp);
            httpd_resp_sendstr_chunk(req, "<propstat><prop>");
            httpd_resp_sendstr_chunk(req, "<resourcetype><collection/></resourcetype>");
            httpd_resp_sendstr_chunk(req, "</prop><status>HTTP/1.1 200 OK</status></propstat></response>");

            while (1) {
                struct dirent *de = readdir(dir);
                if (de == NULL)
                    break;

                struct stat st;
                snprintf(tmp, tmp_size, "%s/%s/%s", MOUNT_POINT, path, de->d_name);
                int result = stat(tmp, &st);
                if (result < 0) {
                    continue;
                }

                httpd_resp_sendstr_chunk(req, "<response>");
                snprintf(tmp, tmp_size, "<href>http://%s%s/%s</href>", host, path, de->d_name);
                httpd_resp_sendstr_chunk(req, tmp);
                httpd_resp_sendstr_chunk(req, "<propstat><prop>");

                if (de->d_type == DT_DIR) {
                    httpd_resp_sendstr_chunk(req, "<resourcetype><collection/></resourcetype>");
                } else {
                    httpd_resp_sendstr_chunk(req, "<resourcetype/>");

                    snprintf(tmp, tmp_size, "<getcontentlength>%lu</getcontentlength>", st.st_size);
                    httpd_resp_sendstr_chunk(req, tmp);
                }

                struct tm tm;
                localtime_r(&st.st_mtim.tv_sec, &tm);
                char strftime_buf[64];
                strftime(strftime_buf, sizeof(strftime_buf), "%a, %d %b %Y %T %z", &tm);
                snprintf(tmp, tmp_size, "<getlastmodified>%s</getlastmodified>", strftime_buf);
                httpd_resp_sendstr_chunk(req, tmp);

                httpd_resp_sendstr_chunk(req, "</prop><status>HTTP/1.1 200 OK</status></propstat></response>");
            }
            closedir(dir);

            httpd_resp_sendstr_chunk(req, "</multistatus>");
            httpd_resp_sendstr_chunk(req, NULL);

        } else {
            httpd_resp_send_404(req);
        }

    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    }

    free(path);
    free(buf);
    free(tmp);

    // ESP_LOG_BUFFER_HEXDUMP(TAG, buf, size, ESP_LOG_INFO);
    return ESP_OK;
}

esp_err_t handler_delete(httpd_req_t *req) {
    const size_t path_size = 512;
    const size_t tmp_size  = 1024;

    char *path = malloc(path_size);
    char *tmp  = malloc(tmp_size);

    if (path && tmp) {
        get_path_from_uri(path, 512, req->uri);
        snprintf(tmp, tmp_size, "%s%s", MOUNT_POINT, path);
        if (unlink(tmp) == 0) {
            httpd_resp_set_status(req, HTTPD_204);
            httpd_resp_send(req, NULL, 0);
        } else {
            if (errno == ENOENT) {
                httpd_resp_set_status(req, HTTPD_404);
                httpd_resp_send(req, NULL, 0);
            } else {
                httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
            }
        }

    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    }

    free(path);
    free(tmp);
    return ESP_OK;
}

esp_err_t handler_get(httpd_req_t *req) {
    const size_t path_size = 512;
    const size_t tmp_size  = 16384;

    char *path = malloc(path_size);
    char *tmp  = malloc(tmp_size);

    if (path && tmp) {
        get_path_from_uri(tmp, tmp_size, req->uri);
        snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);

        FILE *f = fopen(path, "rb");
        if (!f) {
            httpd_resp_set_status(req, HTTPD_404);
            httpd_resp_send(req, NULL, 0);
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

    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    }

    free(path);
    free(tmp);
    return ESP_OK;
}

esp_err_t handler_put(httpd_req_t *req) {
    const size_t path_size = 512;
    const size_t tmp_size  = 16384;

    char *path = malloc(path_size);
    char *tmp  = malloc(tmp_size);

    if (path && tmp) {
        get_path_from_uri(tmp, tmp_size, req->uri);
        snprintf(path, path_size, "%s%s", MOUNT_POINT, tmp);
        printf("PUT %s\n", path);

        FILE *f = fopen(path, "wb");
        if (!f) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to create file");

        } else {
            int remaining = req->content_len;

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

                    ESP_LOGE(TAG, "File reception failed!");
                    // Respond with 500 Internal Server Error
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to receive file");
                    goto done;
                }

                // Write buffer content to file on storage
                if (received && (received != fwrite(tmp, 1, received, f))) {
                    // Couldn't write everything to file! Storage may be full?
                    fclose(f);
                    unlink(path);

                    ESP_LOGE(TAG, "File write failed!");
                    // Respond with 500 Internal Server Error
                    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to write file to storage");
                    goto done;
                }

                // Keep track of remaining size of the file left to be uploaded
                remaining -= received;
            }
            fclose(f);

            httpd_resp_set_status(req, HTTPD_204);
            httpd_resp_send(req, NULL, 0);
        }

    } else {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");
    }

done:
    free(path);
    free(tmp);
    return ESP_OK;
}

esp_err_t download_get_handler(httpd_req_t *req) {
    ESP_LOGI(TAG, "method: %d  uri: %s  content_len: %u", req->method, req->uri, req->content_len);

    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Internal server error.");

    return ESP_FAIL;
}

void fileserver_init(void) {
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Use the URI wildcard matching function in order to
     * allow the same handler to respond to multiple different
     * target URIs which match the wildcard scheme */
    config.uri_match_fn = httpd_uri_match_wildcard;

    ESP_LOGI(TAG, "Starting HTTP Server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start file server!");
        return;
    }

    /* URI handler for getting uploaded files */
    // {
    //     httpd_uri_t file_download = {.uri = "/*", .method = HTTP_GET, .handler = download_get_handler};
    //     httpd_register_uri_handler(server, &file_download);
    // }
    httpd_register_uri_handler(server, &(httpd_uri_t){.uri = "/*", .method = HTTP_DELETE, .handler = handler_delete});
    httpd_register_uri_handler(server, &(httpd_uri_t){.uri = "/*", .method = HTTP_GET, .handler = handler_get});
    httpd_register_uri_handler(server, &(httpd_uri_t){.uri = "/*", .method = HTTP_PUT, .handler = handler_put});
    httpd_register_uri_handler(server, &(httpd_uri_t){.uri = "/*", .method = HTTP_OPTIONS, .handler = handler_options});
    httpd_register_uri_handler(server, &(httpd_uri_t){.uri = "/*", .method = HTTP_PROPFIND, .handler = handler_propfind});
}
