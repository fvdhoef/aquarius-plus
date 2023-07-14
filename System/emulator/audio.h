#pragma once

#define SAMPLES_PER_BUFFER (735)
#define SAMPLERATE (44100)

void      audio_init(void);
void      audio_start(void);
void      audio_close(void);
uint16_t *audio_get_buffer(void);
void      audio_put_buffer(uint16_t *buf);
