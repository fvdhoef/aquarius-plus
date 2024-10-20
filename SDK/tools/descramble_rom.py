#!/bin/env python3
import sys
import argparse

parser = argparse.ArgumentParser(description="Descramble Aquarius ROM file")
parser.add_argument(
    "in_file",
    metavar="in_file.rom",
    help="ROM to descramble",
)
parser.add_argument("out_file", metavar="out_file.rom", help="Descrambled ROM output")
args = parser.parse_args()

with open(args.in_file, "rb") as f:
    data = f.read()

if len(data) != 8192 and len(data) != 16384:
    print("Error: ROM file should be either 8KB or 16KB in size", file=sys.stderr)
    exit(1)

id = data[0:16] if len(data) == 8192 else data[0x2000:0x2010]

# Check constant identifier bytes
if [id[15], id[13], id[11], id[9], id[7], id[5]] != [112, 168, 100, 108, 176, 156]:
    print("Error: Invalid ROM image", file=sys.stderr)
    exit(1)

scramble_code = (
    (674 + (id[3] + id[4] + id[6] + id[8] + id[10] + id[12] + id[14]) + 112) & 0xFF
) ^ 112

out_data = [val ^ scramble_code for val in data]

# Replace header so the descrambled ROM can also be booted
fixup = list(id[0:3]) + [0, 0, 156, 0, 176, 0, 108, 0, 100, 0, 168, 94, 112]
if len(out_data) == 8192:
    out_data = fixup + out_data[16:]
else:
    out_data = out_data[0:8192] + fixup + out_data[8192 + 16 :]


with open(args.out_file, "wb") as f:
    f.write(bytes(out_data))
