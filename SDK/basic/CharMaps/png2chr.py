#!/usr/bin/env python3
import argparse
import PIL.Image

parser = argparse.ArgumentParser(
    description="Convert PNG file to Aq+ CHARRAM file"
)
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))
args = parser.parse_args()

image = PIL.Image.open(args.input)

if image.width != 128 or image.height != 128:
    print("Image must be 128x128")
    exit(1)

# Generate bitmap RAM
bitmapram = bytearray()

for k in range(256):
    for j in range(8):
        val = 0
        for i in range(8):
            if image.getpixel(((k % 16) * 8 + i, (k // 16) * 8 + j))[0] < 128:
                val |= (1<<(7-i))
        bitmapram.append(val)

args.output.write(bitmapram)
