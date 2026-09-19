#include "Midi.h"
#include "MidiData.h"

#ifdef __linux__
#include <alsa/asoundlib.h>
#include <thread>
#endif

Midi::Midi() {
}

Midi *Midi::instance() {
    static Midi obj;
    return &obj;
}

void Midi::init() {
#ifdef __linux__
    std::thread([this] {
        snd_seq_t *seq_handle;
        int        in_port;

        if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_INPUT, 0) < 0 ||
            snd_seq_set_client_name(seq_handle, "Aquarius+ emulator") < 0 ||
            (in_port = snd_seq_create_simple_port(
                 seq_handle, "Master",
                 SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {

            puts("Error initializing ALSA MIDI interface!");
            return;
        }

        auto midiData = MidiData::instance();

        while (1) {
            snd_seq_event_t *ev = NULL;
            snd_seq_event_input(seq_handle, &ev);

            switch (ev->type) {
                case SND_SEQ_EVENT_NOTEOFF: {
                    uint8_t buf[4];
                    buf[0] = 0x08;
                    buf[1] = 0x80 | (ev->data.note.channel & 0xF);
                    buf[2] = ev->data.note.note;
                    buf[3] = ev->data.note.velocity;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_NOTEON: {
                    uint8_t buf[4];
                    buf[0] = 0x09;
                    buf[1] = 0x90 | (ev->data.note.channel & 0xF);
                    buf[2] = ev->data.note.note;
                    buf[3] = ev->data.note.velocity;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_KEYPRESS: {
                    uint8_t buf[4];
                    buf[0] = 0x0A;
                    buf[1] = 0xA0 | (ev->data.note.channel & 0xF);
                    buf[2] = ev->data.note.note;
                    buf[3] = ev->data.note.velocity;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_CONTROLLER: {
                    uint8_t buf[4];
                    buf[0] = 0x0B;
                    buf[1] = 0xB0 | (ev->data.control.channel & 0xF);
                    buf[2] = ev->data.control.param;
                    buf[3] = ev->data.control.value;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_PGMCHANGE: {
                    uint8_t buf[4];
                    buf[0] = 0x0C;
                    buf[1] = 0xC0 | (ev->data.control.channel & 0xF);
                    buf[2] = ev->data.control.value;
                    buf[3] = 0;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_CHANPRESS: {
                    uint8_t buf[4];
                    buf[0] = 0x0D;
                    buf[1] = 0xD0 | (ev->data.control.channel & 0xF);
                    buf[2] = ev->data.control.value;
                    buf[3] = 0;
                    midiData->addData(buf);
                    break;
                }
                case SND_SEQ_EVENT_PITCHBEND: {
                    uint8_t buf[4];
                    buf[0] = 0x0E;
                    buf[1] = 0xE0 | (ev->data.control.channel & 0xF);
                    buf[2] = ev->data.control.value & 0x7F;
                    buf[3] = (ev->data.control.value >> 7) & 0x7F;
                    midiData->addData(buf);
                    break;
                }
                default: {
                    // printf("[%d] Unknown: type=%u\n", ev->time.tick, ev->type);
                    break;
                }
            }
        }
    }).detach();
#endif
}
