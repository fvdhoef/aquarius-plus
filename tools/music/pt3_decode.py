#!/usr/bin/env python3

import argparse
import struct
import array
from sys import stderr

parser = argparse.ArgumentParser()
parser.add_argument("pt3_file", type=argparse.FileType("rb"))

args = parser.parse_args()

pt3_data = args.pt3_file.read()
args.pt3_file.close()

(filetype, name, magic3, author) = struct.unpack_from("30s32s4s33s", pt3_data, 0)

print(f"File size         : {len(pt3_data)}")
print(f"File type         : {filetype.decode()}")
print(f"Name              : {name.decode()}")
print(f"Author            : {author.decode()}")

(
    freq_table,
    speed,
    num_patterns,
    loop_pointer,
    pattern_pointer,
) = struct.unpack_from("<BBBBH", pt3_data, 0x63)

sample_pointers = struct.unpack_from("<32H", pt3_data, 0x69)
ornament_pointers = struct.unpack_from("<16H", pt3_data, 0xA9)
patterns = struct.unpack_from("B" * num_patterns, pt3_data, 0xC9)

print(f"Freq table        : {freq_table}")
print(f"Speed             : {speed}")
print(f"# of patterns     : {num_patterns}")
print(f"Loop pointer      : {loop_pointer}")
print(f"Patterns pointer  : {pattern_pointer}")
print(f"Sample pointers   : {sample_pointers}")
print(f"Ornament pointers : {ornament_pointers}")
print(f"Patterns          : {[pat // 3 for pat in patterns]}")

abc_patterns = [
    struct.unpack_from("<3H", pt3_data, pattern_pointer + patidx * 6)
    for patidx in range(max(patterns) // 3 + 1)
]

pat_offsets = [item for sublist in abc_patterns for item in sublist]
pat_offsets = list(set(pat_offsets))
pat_offsets.sort()

note_names = ["C-", "C#", "D-", "D#", "E-", "F-", "F#", "G-", "G#", "A-", "A#", "B-"]


for offset in pat_offsets:
    print(f"Channel pattern: {offset}")
    effect = 0

    while pt3_data[offset] != 0:
        val = pt3_data[offset]
        note_done = False

        if val >= 0x01 and val <= 0x0F:
            effect = val & 0xF
            print(f"- Effect: {val:02X}")
        elif val == 0x10:
            sample = pt3_data[offset + 1]
            offset += 1
            print(f"- Disable envelope: sample {sample}")
        elif val >= 0x11 and val <= 0x1F:
            envtype = val & 0xF
            envperiod = pt3_data[offset + 1] << 8 | pt3_data[offset + 2]
            envdelay = pt3_data[offset + 3]
            sample = pt3_data[offset + 4]
            offset += 4
            print(
                f"- Set envelope type: {type}, period: {envperiod}, delay: {envdelay}, sample {sample}"
            )
        elif val >= 0x20 and val <= 0x3F:
            noise = val & 0x1F
            print(f"- Set noise: {noise}")
        elif val >= 0x40 and val <= 0x4F:
            ornament = val & 0xF
            print(f"- Set ornament: {ornament}")
        elif val >= 0x50 and val <= 0xAF:
            note = val - 0x50
            print(f"- Play note: {note_names[note % 12]}{note // 12 + 1}")
            note_done = True
        elif val == 0xB0:
            print(f"- Disable envelope, reset ornament position")
        elif val == 0xB1:
            skipval = pt3_data[offset + 1]
            offset += 1
            print(f"- Set skip value: {skipval}")
        elif val >= 0xB2 and val <= 0xBF:
            envtype = (val & 0xF) - 1
            envperiod = pt3_data[offset + 1] << 8 | pt3_data[offset + 2]
            offset += 2
            print(f"- Set envelope type: {envtype}, period: {envperiod}")
        elif val == 0xC0:
            print(f"- Turn off note")
            note_done = True
        elif val >= 0xC1 and val <= 0xCF:
            print(f"- Set volume: {val & 0xF}")
        elif val == 0xD0:
            print(f"- End note")
            note_done = True
        elif val >= 0xD1 and val <= 0xEF:
            sample = val - 0xD0
            print(f"- Set sample: {sample}")
        elif val >= 0xF0 and val <= 0xFF:
            ornament = val & 0xF
            sample = pt3_data[offset + 1]
            offset += 1
            print(f"- Init ornament {ornament} / sample {sample // 2}")
        else:
            print(f"- {val:02X}")

        offset += 1

        if note_done:
            if effect == 0x01:
                delay = pt3_data[offset + 1]
                freq = struct.unpack_from("<h", pt3_data, offset + 2)[0]
                offset += 3
                print(f"* Effect: Glissando  delay: {delay}  freq: {freq}")
            elif effect == 0x02:
                delay = pt3_data[offset + 1]
                unknown = pt3_data[offset + 3] << 8 | pt3_data[offset + 2]
                slide_step = pt3_data[offset + 5] << 8 | pt3_data[offset + 4]
                offset += 5
                print(
                    f"* Effect: Portamento  delay: {delay}  unknown: {unknown}  slide step: {slide_step}"
                )
            elif effect == 0x03:
                sample_offset = pt3_data[offset + 1]
                offset += 1
                print(f"* Effect: Sample offset: {sample_offset}")
            elif effect == 0x04:
                ornament_offset = pt3_data[offset + 1]
                offset += 1
                print(f"* Effect: Ornament offset: {ornament_offset}")
            elif effect == 0x05:
                off_on_delay = pt3_data[offset + 1]
                on_off_delay = pt3_data[offset + 2]
                offset += 2
                print(f"* Effect: Vibrato: {off_on_delay} {on_off_delay}")
            elif effect == 0x08:
                delay = pt3_data[offset + 1]
                slide_add = struct.unpack_from("<h", pt3_data, offset + 2)[0]
                offset += 3
                print(
                    f"* Effect: Envelope Glissando  delay: {delay} slide_add: {slide_add}"
                )
            elif effect == 0x09:
                delay = pt3_data[offset + 1]
                offset += 1
                print(f"* Effect: Set playing speed  delay: {delay}")
