#pragma once

#include "Common.h"
#include <SDL.h>
#include <mutex>

#define SAMPLES_PER_BUFFER (735)
#define SAMPLERATE         (44100)
#define NUM_AUDIO_BUFS     (8)

class Audio {
    Audio();

public:
    static Audio &instance();

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

class DCBlock {
public:
    // https://ccrma.stanford.edu/~jos/fp/DC_Blocker_Software_Implementations.html
    // https://ccrma.stanford.edu/~jos/filters/DC_Blocker.html
    float filter(float x) {
        const float r = 0.995f;
        float       y = x - xm1 + r * ym1;
        xm1           = x;
        ym1           = y;
        return y;
    }
    float xm1 = 0;
    float ym1 = 0;
};
