#!/usr/bin/env python3
from PIL import Image
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("png")
args = parser.parse_args()

img = Image.open(args.png)

assert img.width == 128 and img.height == 48

bin = bytearray()

for k in range(95):
    row = k // 16
    col = k % 16

    for j in range(8):
        val = 0

        for i in range(8):
            if img.getpixel((col * 8 + i, row * 8 + j))[0] != 0:
                val |= 1 << i

        bin.append(val)

print(", ".join([f"0x{x:02x}" for x in bin]))
