#include "Audio.h"

Audio::Audio() {
}

Audio *Audio::instance() {
    static Audio obj;
    return &obj;
}

void Audio::init() {
    if (audioDev > 0) {
        close();
    }

    // Allocate audio buffers
    buffers = (int16_t **)malloc(NUM_AUDIO_BUFS * sizeof(*buffers));
    if (buffers == NULL) {
        fprintf(stderr, "Error allocating audio buffers\n");
        exit(1);
    }
    for (int i = 0; i < NUM_AUDIO_BUFS; i++) {
        buffers[i] = (int16_t *)malloc(2 * SAMPLES_PER_BUFFER * sizeof(buffers[0][0]));
        if (buffers[i] == NULL) {
            fprintf(stderr, "Error allocating audio buffers\n");
            exit(1);
        }
    }

    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;

    // Setup SDL audio
    memset(&desired, 0, sizeof(desired));
    desired.freq     = SAMPLERATE;
    desired.format   = AUDIO_S16SYS;
    desired.samples  = SAMPLES_PER_BUFFER;
    desired.channels = 2;
    desired.callback = _audioCallback;
    desired.userdata = this;

    audioDev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (audioDev <= 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        exit(1);
    }
}

void Audio::_audioCallback(void *userData, uint8_t *stream, int len) {
    static_cast<Audio *>(userData)->audioCallback(stream, len);
}

void Audio::audioCallback(uint8_t *stream, int len) {
    std::lock_guard<std::mutex> lock(mutex);

    assert(len == 2 * SAMPLES_PER_BUFFER * sizeof(buffers[0][0]));

    if (bufCnt <= 0) {
        memset(stream, 0, len);

    } else {
        memcpy(stream, buffers[rdIdx++], len);
        if (rdIdx == NUM_AUDIO_BUFS)
            rdIdx = 0;
        bufCnt--;
    }
}

void Audio::start() {
    // Start playback
    SDL_PauseAudioDevice(audioDev, 0);
}

void Audio::close() {
    if (audioDev <= 0)
        return;

    SDL_CloseAudioDevice(audioDev);
    audioDev = 0;

    // Free audio buffers
    if (buffers != NULL) {
        for (int i = 0; i < NUM_AUDIO_BUFS; i++) {
            if (buffers[i] != NULL) {
                free(buffers[i]);
                buffers[i] = NULL;
            }
        }
        free(buffers);
        buffers = NULL;
    }
}

int16_t *Audio::getBuffer(void) {
    std::lock_guard<std::mutex> lock(mutex);

    if (bufCnt == NUM_AUDIO_BUFS) {
        return NULL;
    }

    // printf("audio_get_buffer %d %d\n", wrIdx, bufCnt);

    assert(bufCnt < NUM_AUDIO_BUFS);
    return buffers[wrIdx];
}

void Audio::putBuffer(int16_t *buf) {
    std::lock_guard<std::mutex> lock(mutex);

    assert(bufCnt < NUM_AUDIO_BUFS);

    // printf("audio_put_buffer %d %d\n", wrIdx, bufCnt);

    buffers[wrIdx++] = buf;
    if (wrIdx == NUM_AUDIO_BUFS)
        wrIdx = 0;
    bufCnt++;
}

int Audio::bufsToRender() {
    std::lock_guard<std::mutex> lock(mutex);
    return NUM_AUDIO_BUFS - bufCnt;
}
