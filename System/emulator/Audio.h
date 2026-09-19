#pragma once

#include "Common.h"
#include <SDL.h>
#include <mutex>
#include "DCBlock.h"

#define SAMPLERATE         (44100)
#define SAMPLES_PER_BUFFER (SAMPLERATE / 60)
#define NUM_AUDIO_BUFS     (8)

class Audio {
    Audio();

public:
    static Audio *instance();

    void     init();
    void     start();
    void     close();
    int16_t *getBuffer();
    void     putBuffer(int16_t *buf);
    int      bufsToRender();

private:
    static void _audioCallback(void *userData, uint8_t *stream, int len);
    void        audioCallback(uint8_t *stream, int len);

    SDL_AudioDeviceID audioDev = 0;
    int16_t         **buffers  = nullptr;
    int               rdIdx    = 0;
    int               wrIdx    = 0;
    volatile int      bufCnt   = 0;
    std::mutex        mutex;
};
