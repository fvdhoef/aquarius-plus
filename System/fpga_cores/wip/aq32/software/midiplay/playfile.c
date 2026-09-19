#include "common.h"
#include "fmsynth.h"
#include "csr.h"

struct chunk_hdr {
    uint8_t  id[4];
    uint32_t length;
};

struct track {
    unsigned       length;
    uint8_t       *data;
    const uint8_t *cur_p;
    const uint8_t *end_p;

    bool     track_done;
    unsigned event_time;
    uint8_t  last_status;
};

static unsigned       tpqn;  // Ticks per quarter note
static unsigned       tempo; // us per quarter note
static unsigned       ticks_per_frame;
static const unsigned t_vsync     = 16666;
static const unsigned ticks_shift = 4;

static void update_ticks_per_frame(void) {
    ticks_per_frame = ((tpqn * t_vsync) << ticks_shift) / tempo;
    printf("Ticks per frame: %d\n", ticks_per_frame);
}

static bool read_varlen(struct track *trk, unsigned *value) {
    unsigned val = 0;

    while (1) {
        if (trk->cur_p >= trk->end_p)
            return false;

        uint8_t b = *(trk->cur_p++);
        val       = (val << 7) | (b & 0x7F);
        if ((b & 0x80) == 0)
            break;
    }

    *value = val;
    return true;
}

static bool parse_meta_event(struct track *trk, uint8_t type, unsigned len) {
    switch (type) {
        case 0x51: {
            // Tempo
            if (len != 3)
                return false;

            tempo = (trk->cur_p[0] << 16) | (trk->cur_p[1] << 8) | trk->cur_p[2];
            printf("[%u] tempo: quarter note duration %u us\n", trk->event_time, tempo);
            update_ticks_per_frame();
            break;
        }
        case 0x58: {
            // Time signature
            if (len != 4)
                return false;

            uint8_t num            = trk->cur_p[0];
            uint8_t denom          = trk->cur_p[1];
            uint8_t clks_per_click = trk->cur_p[2];
            uint8_t bb             = trk->cur_p[3];

            printf("[%u] time signature: num=%u denom=%u metronome=%u bb=%u\n", trk->event_time, num, denom, clks_per_click, bb);
            break;
        }

        case 0x2F: {
            printf("[%u] end of track\n", trk->event_time);
            trk->track_done = true;
            break;
        }

        default: {
            printf("[%u] meta-event %02X %u\n", trk->event_time, type, len);
            break;
        }
    }

    return true;
}

static bool track_parse_delta_time(struct track *trk) {
    if (trk->cur_p >= trk->end_p)
        return false;

    unsigned delta_time;
    if (!read_varlen(trk, &delta_time))
        return false;
    trk->event_time += delta_time;

    return true;
}

static bool track_parse_event(struct track *trk) {
    if (trk->cur_p >= trk->end_p)
        return false;

    uint8_t b = *(trk->cur_p++);
    if (b == 0xFF) {
        uint8_t  type = *(trk->cur_p++);
        unsigned len;
        if (!read_varlen(trk, &len))
            return false;

        const uint8_t *next_p = trk->cur_p + len;
        if (next_p > trk->end_p)
            return false;

        if (!parse_meta_event(trk, type, len))
            return false;

        trk->cur_p = next_p;

    } else {
        if (b & 0x80) {
            trk->last_status = b;
        } else {
            // Running status
            trk->cur_p--;
            b = trk->last_status;
        }

        switch (b & 0xF0) {
            case 0x80: {
                uint8_t channel  = b & 0xF;
                uint8_t note     = *(trk->cur_p++);
                uint8_t velocity = *(trk->cur_p++);
                (void)velocity;
                // printf("[%4u] note off: %u:%u:%u\n", trk->event_time, channel, note, velocity);
                fmsynth_note_off(channel, note);
                break;
            }
            case 0x90: {
                uint8_t channel  = b & 0xF;
                uint8_t note     = *(trk->cur_p++);
                uint8_t velocity = *(trk->cur_p++);
                // printf("[%4u] note on:  %u:%u:%u\n", trk->event_time, channel, note, velocity);
                fmsynth_note_on(channel, note, velocity);
                break;
            }
            case 0xA0: {
                uint8_t channel  = b & 0xF;
                uint8_t note     = *(trk->cur_p++);
                uint8_t pressure = *(trk->cur_p++);
                // printf("[%4u] aftertouch: %u:%u:%u\n", trk->event_time, channel, note, pressure);
                fmsynth_aftertouch(channel, note, pressure);
                break;
            }
            case 0xB0: {
                uint8_t channel    = b & 0xF;
                uint8_t controller = *(trk->cur_p++);
                uint8_t value      = *(trk->cur_p++);
                // printf("[%4u] controller: %u:%u:%u\n", trk->event_time, channel, controller, value);
                fmsynth_controller(channel, controller, value);
                break;
            }
            case 0xC0: {
                uint8_t channel = b & 0xF;
                uint8_t program = *(trk->cur_p++);
                // printf("[%4u] program: %u:%u\n", trk->event_time, channel, program);
                fmsynth_program_change(channel, program);
                break;
            }
            case 0xD0: {
                uint8_t channel  = b & 0xF;
                uint8_t pressure = *(trk->cur_p++);
                // printf("[%4u] channel pressure: %u:%u\n", trk->event_time, channel, pressure);
                fmsynth_channel_pressure(channel, pressure);
                break;
            }
            case 0xE0: {
                uint8_t  channel = b & 0xF;
                uint16_t pitch   = (trk->cur_p[1] << 7) | trk->cur_p[0];
                trk->cur_p += 2;
                // printf("[%4u] pitch: %u:%u\n", trk->event_time, channel, pitch);
                fmsynth_pitch_wheel(channel, pitch);
                break;
            }
            default: {
                switch (b) {
                    // SysEx
                    case 0xF0: {
                        printf("Sysex\n");
                        while ((trk->cur_p[0] & 0x80) == 0) {
                            printf("%02X\n", trk->cur_p[0]);
                            trk->cur_p++;
                        }
                        if (trk->cur_p[0] == 0xF7) {
                            trk->cur_p++;
                        } else {
                            return false;
                        }

                        break;
                    }

                    // MTC Quarter Frame Message
                    case 0xF1: {
                        trk->cur_p += 1;
                        break;
                    }

                    // Song Position Pointer
                    case 0xF2: {
                        trk->cur_p += 2;
                        break;
                    }

                    // Song Select
                    case 0xF3: {
                        trk->cur_p += 1;
                        break;
                    }

                    case 0xF4: break; // Unused
                    case 0xF5: break; // Unused
                    case 0xF6: break; // Tune Request
                    case 0xF7: break; // SysEx end
                    case 0xF8: break; // MIDI Clock
                    case 0xF9: break; // Tick
                    case 0xFA: break; // MIDI Start
                    case 0xFB: break; // MIDI Continue
                    case 0xFC: break; // MIDI Stop
                    case 0xFD: break; // Unused
                    case 0xFE: break; // Active Sense
                    case 0xFF: break; // Reset
                }

                break;
            }
        }
    }
    return true;
}

static void wait_frame(void) {
    // render_audio();
    while ((csr_read_clear(mip, (1 << 16)) & (1 << 16)) == 0);
}

void play_file(const char *path) {
    printf("Playing %s\n", path);
    fmsynth_reset();

    tempo = 500000;

    FILE *f = fopen(path, "rb");
    if (!f) {
        perror(path);
        return;
    }

    struct chunk_hdr hdr;
    if (fread(&hdr, sizeof(hdr), 1, f) != 1)
        goto error;
    hdr.length = __builtin_bswap32(hdr.length);
    if (memcmp(hdr.id, "MThd", 4) != 0 || hdr.length != 6)
        goto error;

    uint16_t format;
    uint16_t num_tracks;
    uint16_t division;

    if (fread(&format, sizeof(format), 1, f) != 1 ||
        fread(&num_tracks, sizeof(num_tracks), 1, f) != 1 ||
        fread(&division, sizeof(division), 1, f) != 1)
        goto error;

    format     = __builtin_bswap16(format);
    num_tracks = __builtin_bswap16(num_tracks);
    division   = __builtin_bswap16(division);

    printf("- Format:       %u\n", format);
    printf("- Track count:  %u\n", num_tracks);
    printf("- Division:     %u\n", division);
    if (num_tracks < 1 || (format != 0 && format != 1)) {
        printf("Incompatible format!\n");
        goto error;
    }
    if (division & 0x8000) {
        printf("SMPTE format is unsupported\n");
        goto error;
    }
    tpqn = division;

    update_ticks_per_frame();

    struct track *tracks = calloc(num_tracks, sizeof(struct track));
    if (!tracks)
        goto error;

    for (unsigned i = 0; i < num_tracks; i++) {
        struct track *trk = &tracks[i];

        if (fread(&hdr, sizeof(hdr), 1, f) != 1)
            goto error;
        hdr.length = __builtin_bswap32(hdr.length);
        if (memcmp(hdr.id, "MTrk", 4) != 0)
            goto error;

        printf("- Track %u length: %u\n", i, (unsigned)hdr.length);

        trk->length = hdr.length;
        trk->data   = malloc(hdr.length);
        if (!trk->data)
            goto error;
        trk->cur_p = trk->data;
        trk->end_p = trk->data + trk->length;

        if (fread(trk->data, trk->length, 1, f) != 1)
            goto error;
    }
    fclose(f);

    unsigned t = 0;

    for (unsigned i = 0; i < num_tracks; i++) {
        if (!track_parse_delta_time(&tracks[i]))
            return;
    }

    while (1) {
        unsigned cur_ticks = t >> ticks_shift;

        bool stop = true;

        for (unsigned i = 0; i < num_tracks; i++) {
            struct track *trk = &tracks[i];

            if (!trk->track_done)
                stop = false;

            while (!trk->track_done && trk->event_time <= cur_ticks) {
                if (!track_parse_event(trk)) {
                    trk->track_done = true;
                    break;
                }
                if (!track_parse_delta_time(trk)) {
                    trk->track_done = true;
                    break;
                }
            }
        }

        // Wait for delta time
        t += ticks_per_frame;
        wait_frame();

        if (stop)
            break;
    }

    // Render for another second
    for (int i = 0; i < 60; i++) {
        wait_frame();
    }

    return;

error:
    printf("Error!\n");
    return;
}
