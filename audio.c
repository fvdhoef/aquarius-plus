#include "common.h"
#include "audio.h"
#include <SDL.h>

#define NUM_BUFS (64)

static SDL_AudioDeviceID audio_dev;
static int16_t **        buffers;
static int               rdidx   = 0;
static int               wridx   = 0;
static volatile int      buf_cnt = 0;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;

    assert(len == SAMPLES_PER_BUFFER * sizeof(buffers[0][0]));

    SDL_Event event;
    memset(&event, 0, sizeof(event));
    event.type      = SDL_USEREVENT;
    event.user.type = SDL_USEREVENT;
    event.user.code = 0;
    SDL_PushEvent(&event);

    if (buf_cnt <= 0) {
        memset(stream, 0, len);

    } else {
        memcpy(stream, buffers[rdidx++], len);
        if (rdidx == NUM_BUFS)
            rdidx = 0;
        buf_cnt--;
    }
}

void audio_init(void) {
    if (audio_dev > 0) {
        audio_close();
    }

    // Allocate audio buffers
    buffers = malloc(NUM_BUFS * sizeof(*buffers));
    for (int i = 0; i < NUM_BUFS; i++) {
        buffers[i] = malloc(SAMPLES_PER_BUFFER * sizeof(buffers[0][0]));
    }

    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;

    // Setup SDL audio
    memset(&desired, 0, sizeof(desired));
    desired.freq     = SAMPLERATE;
    desired.format   = AUDIO_S16SYS;
    desired.samples  = SAMPLES_PER_BUFFER;
    desired.channels = 1;
    desired.callback = audio_callback;

    audio_dev = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    if (audio_dev <= 0) {
        fprintf(stderr, "SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
        exit(1);
    }
}

void audio_start(void) {
    // Start playback
    SDL_PauseAudioDevice(audio_dev, 0);
}

void audio_close(void) {
    if (audio_dev <= 0)
        return;

    SDL_CloseAudioDevice(audio_dev);
    audio_dev = 0;

    // Free audio buffers
    if (buffers != NULL) {
        for (int i = 0; i < NUM_BUFS; i++) {
            if (buffers[i] != NULL) {
                free(buffers[i]);
                buffers[i] = NULL;
            }
        }
        free(buffers);
        buffers = NULL;
    }
}

int16_t *audio_get_buffer(void) {
    if (buf_cnt == NUM_BUFS) {
        return NULL;
    }

    // printf("audio_get_buffer %d %d\n", wridx, buf_cnt);

    assert(buf_cnt < NUM_BUFS);
    return buffers[wridx];
}

void audio_put_buffer(int16_t *buf) {
    assert(buf_cnt < NUM_BUFS);

    // printf("audio_put_buffer %d %d\n", wridx, buf_cnt);

    buffers[wridx++] = buf;
    if (wridx == NUM_BUFS)
        wridx = 0;
    buf_cnt++;
}
