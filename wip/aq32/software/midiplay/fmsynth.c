#include "fmsynth.h"
#include "instrument_bank.h"

#define NUM_FM_CHANNELS 32

struct fm_channel {
    int8_t midi_ch;
    int8_t midi_note;
    int8_t midi_velocity;

    uint8_t age;
    bool    is_4op;

    bool                     apply_volume[2];
    const struct instrument *instrument;
};

struct midi_channel {
    const struct instrument *instrument;
    uint8_t                  bank;       // Controller 0
    uint8_t                  volume;     // Controller 7
    uint8_t                  expression; // Controller 11
    uint8_t                  pan;        // Controller 10
};

static uint8_t master_volume = 127;

struct op_settings {
    uint8_t a, d, s, r;
    uint8_t ws;
    bool    am;
    bool    ksr;
    bool    vib;
    bool    sus;
    uint8_t mult;
    uint8_t ksl;
    uint8_t tl;
};

static struct midi_channel midi_channels[16];
static struct fm_channel   fm_channels[NUM_FM_CHANNELS];

static const uint16_t fnums[19] = {
    346, 367, 389, 412, 436, 462, 490, 519, 550, 582, 617, 654,
    692, 733, 777, 823, 872, 924, 979};

static const uint8_t s_w9x_generic_fm_volume_model[32] = {
    40, 36, 32, 28, 23, 21, 19, 17,
    15, 14, 13, 12, 11, 10, 9, 8,
    7, 6, 5, 5, 4, 4, 3, 3,
    2, 2, 1, 1, 1, 0, 0, 0};

static int find_channel(bool is_4op) {
    int result = -1;
    int age    = -1;

    if (is_4op) {
        for (int ch = 0; ch < NUM_FM_CHANNELS; ch += 2) {
            if (fm_channels[ch].midi_ch < 0 && fm_channels[ch + 1].midi_ch < 0) {
                if (fm_channels[ch].age > age) {
                    age    = fm_channels[ch].age;
                    result = ch;
                }
            }
        }
    } else {
        for (int ch = 0; ch < NUM_FM_CHANNELS; ch++) {
            if (fm_channels[ch].midi_ch < 0) {
                if (fm_channels[ch].age > age) {
                    age    = fm_channels[ch].age;
                    result = ch;
                }
            }
        }
    }
    return result;
}

void fmsynth_reset(void) {
    for (int i = 0; i < NUM_FM_CHANNELS; i++)
        fm_channels[i].midi_ch = -1;

    FMSYNTH->ctrl = (1 << 7) | (1 << 6);

    for (int i = 0; i < 16; i++) {
        midi_channels[i].bank       = 0;
        midi_channels[i].instrument = instrument_bank;
        midi_channels[i].volume     = 127;
        midi_channels[i].expression = 127;
        midi_channels[i].pan        = 64;
    }
}

void fmsynth_note_off(uint8_t channel, uint8_t note) {
    if (channel > 15 || note > 127)
        return;

    unsigned ch;
    for (ch = 0; ch < NUM_FM_CHANNELS; ch++)
        if (fm_channels[ch].midi_ch == channel && fm_channels[ch].midi_note == note)
            break;
    if (ch >= NUM_FM_CHANNELS)
        return;

    // printf("ch%d: note %3d off\n", channel, note);

    bool is_4op = fm_channels[ch].is_4op;

    if (is_4op && (ch & 1) != 0)
        printf("Ieks!\n");

    write_key_off(is_4op ? (3 << ch) : (1 << ch));

    fm_channels[ch].midi_ch       = -1;
    fm_channels[ch].midi_note     = -1;
    fm_channels[ch].midi_velocity = -1;
    fm_channels[ch].age           = 0;
    fm_channels[ch].is_4op        = false;

    if (is_4op) {
        fm_channels[ch | 1].midi_ch       = -1;
        fm_channels[ch | 1].midi_note     = -1;
        fm_channels[ch | 1].midi_velocity = -1;
        fm_channels[ch | 1].age           = 0;
        fm_channels[ch | 1].is_4op        = false;
    }

    for (int i = 0; i < NUM_FM_CHANNELS; i++) {
        if (fm_channels[i].midi_ch < 0 && fm_channels[i].age < 127)
            fm_channels[i].age++;
    }
}

static uint16_t calculate_block_fnum(uint8_t note) {
    int octave = (note / 12) - 1;
    if (octave < 0)
        return 0;
    if (octave > 7)
        octave = 7;

    int fnum_idx = note - (octave + 1) * 12;
    if (fnum_idx > 18)
        return 0;

    return octave << 10 | fnums[fnum_idx];
}

static uint32_t op_attr0_apply_volume(uint32_t op_attr0, unsigned volume) {
    unsigned tl = (op_attr0 >> 16) & 63;
    tl += volume;
    if (tl > 0x3F)
        tl = 0x3F;
    return (op_attr0 & ~(63 << 16)) | (tl << 16);
}

static unsigned calc_volume(unsigned channel, unsigned velocity) {
    struct midi_channel *midi_channel = &midi_channels[channel];

    unsigned volume = (midi_channel->volume * midi_channel->expression * master_volume) / 16129;
    volume          = s_w9x_generic_fm_volume_model[volume >> 2];
    // if (volume > 63)
    //     volume = 63;

    volume += s_w9x_generic_fm_volume_model[velocity >> 2];
    return volume;
}

void fmsynth_note_on(uint8_t channel, uint8_t midi_note, uint8_t velocity) {
    if (channel > 15 || midi_note > 127)
        return;

    fmsynth_note_off(channel, midi_note);
    if (velocity == 0)
        return;

    struct midi_channel     *midi_channel = &midi_channels[channel];
    const struct instrument *instrument   = midi_channel->instrument;

    if (channel == 9) {
        // Percussion
        instrument = &instrument_bank[128 + midi_note];
    }

    // printf("ch%d: note %3d on, velocity=%u  instrument=%p  expression=%u\n", channel, midi_note, velocity, (void *)instrument, midi_channel->expression);

    if (instrument == NULL || instrument->flags == 4)
        return;

    bool is_4op      = instrument->flags == 1;
    bool is_pseudo_4 = instrument->flags == 3;

    int fmch = find_channel(is_4op || is_pseudo_4);
    if (fmch < 0) {
        printf("Out of channels!\n");
        return;
    }

    unsigned notenr = midi_note;
    if (instrument->perc_note != 0)
        notenr = instrument->perc_note;

    uint16_t blk_fnum0 = calculate_block_fnum(notenr + instrument->note_offset[0]);
    if (blk_fnum0 == 0)
        return;

    unsigned volume = calc_volume(channel, velocity);

    unsigned alg;
    if (!is_4op) {
        alg = instrument->fb_alg[0] & 1;
    } else {
        alg = 2 + (((instrument->fb_alg[0] & 1) << 1) | (instrument->fb_alg[1] & 1));
    }

    static const bool apply_volume[6][4] = {
        {0, 1, 0, 1}, // 2-op alg0
        {1, 1, 1, 1}, // 2-op alg1
        {0, 0, 0, 1}, // 4-op alg0
        {0, 1, 0, 1}, // 4-op alg1
        {1, 0, 0, 1}, // 4-op alg2
        {1, 0, 1, 1}, // 4-op alg3
    };

    write_4op(fmch, is_4op);

    if (is_4op || is_pseudo_4) {
        uint16_t blk_fnum1 = calculate_block_fnum(notenr + instrument->note_offset[is_4op ? 0 : 1]);
        if (blk_fnum1 == 0)
            return;

        if ((fmch & 1) != 0)
            printf("Ieks2!\n");

        fm_channels[fmch].midi_ch             = channel;
        fm_channels[fmch].midi_note           = midi_note;
        fm_channels[fmch].midi_velocity       = velocity;
        fm_channels[fmch].is_4op              = true;
        fm_channels[fmch].apply_volume[0]     = apply_volume[alg][0];
        fm_channels[fmch].apply_volume[1]     = apply_volume[alg][1];
        fm_channels[fmch].instrument          = instrument;
        fm_channels[fmch + 1].midi_ch         = channel;
        fm_channels[fmch + 1].midi_note       = midi_note;
        fm_channels[fmch + 1].midi_velocity   = velocity;
        fm_channels[fmch + 1].is_4op          = true;
        fm_channels[fmch + 1].apply_volume[0] = apply_volume[alg][2];
        fm_channels[fmch + 1].apply_volume[1] = apply_volume[alg][3];
        fm_channels[fmch + 1].instrument      = instrument;

        for (int i = 0; i < 4; i++) {
            write_op(
                fmch * 2 + i,
                apply_volume[alg][i] ? op_attr0_apply_volume(instrument->op_attr0[i], volume) : instrument->op_attr0[i],
                instrument->op_attr1[i]);
        }
        write_ch(fmch + 0, (midi_channel->pan << 20) | (instrument->fb_alg[0] << 16) | blk_fnum0);
        write_ch(fmch + 1, (midi_channel->pan << 20) | (instrument->fb_alg[1] << 16) | blk_fnum1);
        write_key_on(3 << fmch);

    } else {
        fm_channels[fmch].midi_ch         = channel;
        fm_channels[fmch].midi_note       = midi_note;
        fm_channels[fmch].midi_velocity   = velocity;
        fm_channels[fmch].is_4op          = false;
        fm_channels[fmch].apply_volume[0] = apply_volume[alg][0];
        fm_channels[fmch].apply_volume[1] = apply_volume[alg][1];
        fm_channels[fmch].instrument      = instrument;

        for (int i = 0; i < 2; i++) {
            write_op(
                fmch * 2 + i,
                apply_volume[alg][i] ? op_attr0_apply_volume(instrument->op_attr0[i], volume) : instrument->op_attr0[i],
                instrument->op_attr1[i]);
        }

        write_ch(fmch, (midi_channel->pan << 20) | (instrument->fb_alg[0] << 16) | blk_fnum0);
        write_key_on(1 << fmch);
    }
}

static void update_channel_volume(uint8_t channel) {
    for (int fmch = 0; fmch < 32; fmch++) {
        struct fm_channel *chan = &fm_channels[fmch];
        if (chan->midi_ch != channel)
            continue;

        unsigned volume = calc_volume(channel, chan->midi_velocity);

        // Update volume
        for (int i = 0; i < 2; i++) {
            if (!chan->apply_volume[i])
                continue;

            unsigned op_attr_idx = ((fmch & 1) << 1) + i;
            write_op(fmch * 2 + i, op_attr0_apply_volume(chan->instrument->op_attr0[op_attr_idx], volume), chan->instrument->op_attr1[op_attr_idx]);
        }
    }
}

void fmsynth_aftertouch(uint8_t channel, uint8_t note, uint8_t pressure) {
    // printf("ch%d: aftertouch %3d pressure=%u\n", channel, note, pressure);
}

void fmsynth_controller(uint8_t channel, uint8_t controller, uint8_t value) {
    if (channel > 15 || controller > 127 || value > 127)
        return;

    switch (controller) {
        case 0: midi_channels[channel].bank = value; break;
        case 7:
            printf("ch%d: volume %u\n", channel, value);
            midi_channels[channel].volume = value;
            update_channel_volume(channel);
            break;
        case 10: {
            printf("ch%d: pan %u\n", channel, value);
            if (value < 1)
                value = 1;
            midi_channels[channel].pan = value;
            break;
        }
        case 11: {
            printf("ch%d: expression %u\n", channel, value);
            midi_channels[channel].expression = value;
            update_channel_volume(channel);
            break;
        }
        case 120:   // All sound off
        case 123: { // All notes off
            for (int ch = 0; ch < NUM_FM_CHANNELS; ch++) {
                if (fm_channels[ch].midi_ch == channel) {
                    write_key_off(1 << ch);
                    fm_channels[ch].midi_ch = -1;
                }
            }
            break;
        }

        default:
            printf("ch%d: controller %u: %u\n", channel, controller, value);
            break;
    }
}

void fmsynth_program_change(uint8_t channel, uint8_t program) {
    if (channel > 15 || program > 127)
        return;

    printf("ch%d: program change %u\n", channel, program);
    if (channel == 9)
        return;

    // if (midi_channels[channel].bank == 0) {
    midi_channels[channel].instrument = &instrument_bank[program];
    // } else {
    //     midi_channels[channel].instrument = NULL;
    // }
}

void fmsynth_channel_pressure(uint8_t channel, uint8_t pressure) {
    // printf("ch%d: channel_pressure %u\n", channel, pressure);
}

void fmsynth_pitch_wheel(uint8_t channel, uint16_t pitch) {
    // printf("ch%d: pitch wheel %u\n", channel, pitch);
}
