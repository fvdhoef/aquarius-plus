#pragma once

#include "common.h"

void write_op(unsigned idx, uint32_t attr0, uint32_t attr1);
void write_ch(unsigned idx, uint32_t attr);
void write_4op(unsigned idx, bool on);
void write_key_on(unsigned mask);
void write_key_off(unsigned mask);

void fmsynth_reset(void);
void fmsynth_note_off(uint8_t channel, uint8_t note);
void fmsynth_note_on(uint8_t channel, uint8_t note, uint8_t velocity);
void fmsynth_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure);
void fmsynth_controller(uint8_t channel, uint8_t controller, uint8_t value);
void fmsynth_program_change(uint8_t channel, uint8_t program);
void fmsynth_channel_pressure(uint8_t channel, uint8_t pressure);
void fmsynth_pitch_wheel(uint8_t channel, uint16_t pitch);
