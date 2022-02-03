#pragma once

#define SAMPLES_PER_BUFFER (800)
#define SAMPLERATE (48000)

void     audio_init(void);
void     audio_start(void);
void     audio_close(void);
int16_t *audio_get_buffer(void);
void     audio_put_buffer(int16_t *buf);
