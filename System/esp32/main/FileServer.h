#pragma once

#include "Common.h"
#include <esp_http_server.h>

class FileServer {
    FileServer();

public:
    static FileServer &instance();
    void               init();

private:
    using HttpHandler = std::function<esp_err_t(httpd_req_t *)>;
    void             registerHandler(const std::string &uri, httpd_method_t, HttpHandler);
    static esp_err_t _handler(httpd_req_t *);

    esp_err_t postKeyboard(httpd_req_t *);
    esp_err_t postSysRom(httpd_req_t *);
    esp_err_t handleDelete(httpd_req_t *);
    esp_err_t handleGet(httpd_req_t *);
    esp_err_t handlePut(httpd_req_t *);
    esp_err_t handleCopy(httpd_req_t *);
    esp_err_t handleMkCol(httpd_req_t *);
    esp_err_t handleMove(httpd_req_t *);
    esp_err_t handleOptions(httpd_req_t *);
    esp_err_t handlePropFind(httpd_req_t *);

    struct HandlerContext {
        HttpHandler handler;
    };
    httpd_handle_t server;
};
