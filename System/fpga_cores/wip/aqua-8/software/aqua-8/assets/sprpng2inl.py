#!/usr/bin/env python3
from PIL import Image
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("png")
args = parser.parse_args()

img = Image.open(args.png)

assert (
    img.width == 128
    and img.height == 32
    and img.mode == "P"
    and len(img.getpalette()) == 48
)

for j in range(32):
    values = []
    for i in range(128 // 8):
        val = img.getpixel((i * 8 + 0, j)) << 0
        val |= img.getpixel((i * 8 + 1, j)) << 4
        val |= img.getpixel((i * 8 + 2, j)) << 8
        val |= img.getpixel((i * 8 + 3, j)) << 12
        val |= img.getpixel((i * 8 + 4, j)) << 16
        val |= img.getpixel((i * 8 + 5, j)) << 20
        val |= img.getpixel((i * 8 + 6, j)) << 24
        val |= img.getpixel((i * 8 + 7, j)) << 28
        values.append(val)

    print(" ".join([f"0x{val:08x}," for val in values]))
