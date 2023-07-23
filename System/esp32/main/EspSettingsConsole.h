#include "common.h"
#include <freertos/stream_buffer.h>

class EspSettingsConsole {
    EspSettingsConsole();

public:
    static EspSettingsConsole &instance();

    void init();
    void newSession();
    int  recv(void *buf, size_t size, TickType_t ticksToWait);
    int  send(const void *buf, size_t size, TickType_t ticksToWait);

private:
    StreamBufferHandle_t tx_buffer;
    StreamBufferHandle_t rx_buffer;
    volatile bool        new_session;

    static void _consoleTask(void *);
    void        consoleTask();

    void cprintf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void cputc(char ch);
    char cgetc();
    void creadline(char *buf, size_t max_len, bool is_password);
    void showHelp();
    void wifiStatus();
    void wifiSet();
    void showDate();
    void systemUpdate();
};
