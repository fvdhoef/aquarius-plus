#!/usr/bin/env python3
from PIL import Image
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("bin")
parser.add_argument("png")
args = parser.parse_args()

img = Image.new("RGB", (128, 48), "black")

with open(args.bin, "rb") as f:
    font = f.read()
    assert len(font) == 760

    for k in range(95):
        row = k // 16
        col = k % 16

        for j in range(8):
            for i in range(8):
                if (font[k * 8 + j] & (1 << i)) != 0:
                    img.putpixel((col * 8 + i, row * 8 + j), (255, 255, 255))

img.save(args.png)
