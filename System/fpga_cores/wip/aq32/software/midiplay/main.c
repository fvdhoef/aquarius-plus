#include "common.h"
#include "esp.h"
#include "fmsynth.h"

static uint8_t buf[256];

void write_op(unsigned idx, uint32_t attr0, uint32_t attr1) {
    FMSYNTH->op_attr0[idx] = attr0;
    FMSYNTH->op_attr1[idx] = attr1;
}

void write_ch(unsigned idx, uint32_t attr) {
    FMSYNTH->ch_attr[idx] = attr;
}

void write_4op(unsigned idx, bool on) {
    if (on)
        FMSYNTH->opmode |= 1 << (idx / 2);
    else
        FMSYNTH->opmode &= ~(1 << (idx / 2));
}

void write_key_on(unsigned mask) {
    FMSYNTH->key_on |= mask;
}

void write_key_off(unsigned mask) {
    FMSYNTH->key_on &= ~mask;
}

void usb_midi(void) {
    printf("Receiving data from USB MIDI...\n");

    while (1) {
        int count = esp_get_midi_data(buf, sizeof(buf));
        count /= 4;

        const uint8_t *p = buf;

        for (int i = 0; i < count; i++) {
            // printf("%02X %02X %02X %02X\n", p[0], p[1], p[2], p[3]);

            uint8_t channel = p[1] & 0xF;
            switch (p[1] >> 4) {
                case 0x8: fmsynth_note_off(channel, p[2]); break;
                case 0x9: fmsynth_note_on(channel, p[2], p[3]); break;
                case 0xA: fmsynth_aftertouch(channel, p[2], p[3]); break;
                case 0xB: fmsynth_controller(channel, p[2], p[3]); break;
                case 0xC: fmsynth_program_change(channel, p[2]); break;
                case 0xD: fmsynth_channel_pressure(channel, p[2]); break;
                case 0xE: fmsynth_pitch_wheel(channel, (p[3] << 7) | p[2]); break;
                case 0xF: {
                    switch (p[1] & 0xF) {
                        case 0xF: fmsynth_reset(); break;
                        default: {
                            printf("%02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
                            break;
                        }
                    }
                    break;
                }
                default:
                    printf("%02x %02x %02x %02x\n", p[0], p[1], p[2], p[3]);
                    break;
            }
            p += 4;
        }
    }
}

extern void play_file(const char *path);

int main(int argc, const char **argv) {
    printf("Aquarius32 MIDI player\n\n");
    fmsynth_reset();

    if (argc == 1) {
        usb_midi();
    } else if (argc == 2) {
        play_file(argv[1]);
    } else {
        printf("midiplay.aq32 [<song.mid>]\n");
    }

    fmsynth_reset();
    return 0;
}
