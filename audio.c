#include "common.h"
#include "audio.h"
#include <SDL.h>

#define SAMPLERATE (48000)
#define SAMPLES_PER_BUFFER (800)

struct entry {
    double  t;
    uint8_t val;
};

#define NUM_ENTRIES (1024)

static struct entry entries[NUM_ENTRIES];
static int          entries_wridx = 0;
static int          entries_rdidx = 0;
static volatile int entries_cnt   = 0;

static SDL_AudioDeviceID audio_dev;

static struct entry current_entry;

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    (void)userdata;

    static double render_t = 0;

    int expected = SAMPLES_PER_BUFFER * sizeof(int16_t);
    if (len != expected) {
        printf("Audio buffer size mismatch! (expected: %d, got: %d)\n", expected, len);
        return;
    }

    double t_per_sample = 1.0f / SAMPLERATE;

    int16_t *buf = (int16_t *)stream;
    int      i   = 0;

    while (i < SAMPLES_PER_BUFFER) {
        if (entries_cnt > 0) {
            // Check next entry
            do {
                if (entries_cnt == 0)
                    break;

                static bool first = true;
                if (first) {
                    render_t = entries[entries_rdidx].t - 0.1;
                    first    = false;
                }

                struct entry next_entry = entries[entries_rdidx];
                if (render_t >= next_entry.t) {
                    current_entry = next_entry;
                    entries_rdidx++;
                    if (entries_rdidx == NUM_ENTRIES)
                        entries_rdidx = 0;
                    entries_cnt--;
                } else {
                    break;
                }
            } while (render_t > current_entry.t);
            // printf("%lf %lf\n", render_t, current_entry.t);
        }

        buf[i++] = current_entry.val ? 4095 : -4095;
        render_t += t_per_sample;

        // current_entry.delta_tstates -= tstates_per_sample;
    }
}

void audio_init(void) {
    if (audio_dev > 0) {
        audio_close();
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
        return;
    }

    // Start playback
    SDL_PauseAudioDevice(audio_dev, 0);
}

void audio_close(void) {
    if (audio_dev <= 0)
        return;

    SDL_CloseAudioDevice(audio_dev);
    audio_dev = 0;
}

void audio_set(int cpu_cycles, unsigned val) {
    if (entries_cnt < NUM_ENTRIES) {
        entries[entries_wridx].t   = (double)cpu_cycles / (double)CPU_FREQ;
        entries[entries_wridx].val = val;
        entries_wridx++;
        if (entries_wridx == NUM_ENTRIES)
            entries_wridx = 0;
        entries_cnt++;
    } else {
        printf("audio_set: Out of entries!\n");
    }
}
