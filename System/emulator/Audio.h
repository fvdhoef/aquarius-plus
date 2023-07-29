#pragma once

#include "common.h"
#include <SDL.h>

#define SAMPLES_PER_BUFFER (735)
#define SAMPLERATE (44100)

class Audio {
    Audio();

public:
    static Audio &instance();

    void      init();
    void      start();
    void      close();
    uint16_t *getBuffer();
    void      putBuffer(uint16_t *buf);

private:
    static void _audioCallback(void *userData, uint8_t *stream, int len);
    void        audioCallback(uint8_t *stream, int len);

    SDL_AudioDeviceID audioDev = 0;
    uint16_t        **buffers  = nullptr;
    int               rdIdx    = 0;
    int               wrIdx    = 0;
    volatile int      bufCnt   = 0;
};
