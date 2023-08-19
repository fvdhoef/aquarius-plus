// This file is shared between the emulator and ESP32. It needs to be manually copied when changed.
#include "Common.h"
#ifndef EMULATOR
#    include <freertos/stream_buffer.h>
#else
#    include <thread>
#    include "Queue.h"
#endif

class EspSettingsConsole {
    EspSettingsConsole();

public:
    static EspSettingsConsole &instance();

    void init();
    void newSession();
    int  recv(void *buf, size_t size);
    int  send(const void *buf, size_t size);

private:
#ifndef EMULATOR
    StreamBufferHandle_t tx_buffer = nullptr;
    StreamBufferHandle_t rx_buffer = nullptr;
#else
    Queue<uint8_t> tx_buffer;
    Queue<uint8_t> rx_buffer;
#endif
    volatile bool new_session = true;

#ifndef EMULATOR
    static void _consoleTask(void *);
#endif
    void consoleTask();

    void cprintf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
    void cputc(char ch);
    char cgetc();
    void creadline(char *buf, size_t max_len, bool is_password);
    void showHelp();
    void wifiStatus();
    void wifiSet();
    void showDate();
    void systemUpdate();
    void systemUpdateGitHub();
};
