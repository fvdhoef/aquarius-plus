#!/usr/bin/env python3
import struct

with open("GM-By-J.A.Nguyen-and-Wohlstand.wopl", "rb") as f:
# with open("msadlib.wopl", "rb") as f:
    wopldata = f.read()

if wopldata[0:11] != b"WOPL3-BANK\x00":
    print("Invalid file")
    exit(1)

offset = 11


def get(format):
    global offset
    result = struct.unpack_from(format, wopldata, offset)
    offset += struct.calcsize(format)
    return result


(version,) = get("<H")
(num_mbanks, num_pbanks, globl_flags, volume_scaling) = get(">HHBB")
print(
    f"version={version} num_mbanks={num_mbanks} num_pbanks={num_pbanks} globl_flags={globl_flags:02x} volume_scaling={volume_scaling}"
)

if version >= 2:
    print("Melodic banks:")
    for i in range(num_mbanks):
        (name, bank_lsb, bank_msb) = get("32sBB")
        name = name.split(b"\0", 1)[0].decode()
        print(f"{i}: name='{name}' bank_lsb={bank_lsb} bank_msb={bank_msb}")

    print("Percussion banks:")
    for i in range(num_pbanks):
        (name, bank_lsb, bank_msb) = get("32sBB")
        name = name.split(b"\0", 1)[0].decode()
        print(f"{i}: name='{name}' bank_lsb={bank_lsb} bank_msb={bank_msb}")

if num_mbanks != 1 or num_pbanks != 1:
    print("Unsupported bank count")


output = bytearray()

f = open("../instrument_bank.h", "wt")
print(
    """#pragma once

#include <stdint.h>

struct instrument {
    int8_t   note_offset[2];
    int8_t   ch2_detune;
    uint8_t  perc_note;
    uint8_t  flags;
    uint8_t  fb_alg[2];
    uint32_t op_attr0[4];
    uint8_t  op_attr1[4];
};

// clang-format off
static const struct instrument instrument_bank[256] = {""",
    file=f,
)

for i in range(256):
    key_on_delay_ms = 0
    key_off_delay_ms = 0

    (
        name,
        note_offset1,
        note_offset2,
        velocity_offset,
        voice_detune1,
        perc_note,
        flags,
        fb_alg1,
        fb_alg2,
    ) = get(">32shhbbBBBB")
    name = name.split(b"\0", 1)[0].decode()

    output.extend(
        struct.pack(
            "bbbBBBB",
            note_offset1,
            note_offset2,
            voice_detune1,
            perc_note,
            flags,
            fb_alg1,
            fb_alg2,
        )
    )

    op_attr0 = []
    op_attr1 = []

    for j in range(4):
        (am_vib_sus_ksr_mult, ksl_tl, ad, sr, wf) = get("BBBBB")

        output.extend(struct.pack("BBBBB", sr, ad, ksl_tl, am_vib_sus_ksr_mult, wf))

        op_attr1.append(wf)
        op_attr0.append((am_vib_sus_ksr_mult << 24) | (ksl_tl << 16) | (ad << 8) | sr)
        # print(f"  {j}: op_attr0={op_attr0:08x} op_attr1 = {op_attr1:02x}")

    print(
        f"    /* {i:3} */ {{.note_offset = {{{note_offset1:4}, {note_offset2:4}}}, .ch2_detune = {voice_detune1:4}, .perc_note = {perc_note:3}, .flags = {flags}, .fb_alg = {{0x{fb_alg1:x}, 0x{fb_alg2:x}}}, .op_attr0 = {{0x{op_attr0[1]:08x}, 0x{op_attr0[0]:08x}, 0x{op_attr0[3]:08x}, 0x{op_attr0[2]:08x}}}, .op_attr1 = {{0x{op_attr1[1]:02x}, 0x{op_attr1[0]:02x}, 0x{op_attr1[3]:02x}, 0x{op_attr1[2]:02x}}}}},",
        file=f,
    )

    if version >= 3:
        (key_on_delay_ms, key_off_delay_ms) = get(">HH")


print(len(output))

print(
    """};
// clang-format on""",
    file=f,
)
f.close()
