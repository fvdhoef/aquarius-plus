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
    const char    *cur_tz = "";
#endif
    volatile bool new_session = true;

#ifndef EMULATOR
    static void _consoleTask(void *);
#endif
    void consoleTask();

#if _WIN32
    void cprintf(const char *fmt, ...);
#else
    void cprintf(const char *fmt, ...) __attribute__((format(printf, 2, 3)));
#endif
    void cputc(char ch);
    char cgetc();
    void creadline(char *buf, size_t max_len, bool is_password);
    bool creadUint(unsigned *value);
    void showHelp();
    void wifiStatus();
    void wifiSet();
    void showDate();
    void timeZoneShow();
    void timeZoneSet();
    void systemUpdate();
    void systemUpdateGitHub();
};
