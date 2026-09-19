#!/usr/bin/env python3
import argparse
import PIL.Image

parser = argparse.ArgumentParser(description="Convert image file to Aq+ bitmap file")
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))
args = parser.parse_args()

image = PIL.Image.open(args.input)

if image.width != 160 or image.height != 200:
    print("Image must be 160x200")
    exit(1)

image = image.convert(mode="P", palette=PIL.Image.ADAPTIVE, colors=16, dither=PIL.Image.FLOYDSTEINBERG)

palette = image.getpalette()
palette = [palette[3 * n : 3 * n + 3] for n in range(256)][0 : len(image.getcolors())]
if len(palette) > 16:
    print("Too many colors in picture")
    exit(1)

palette = [
    ((entry[0] >> 4) << 8) | ((entry[1] >> 4) << 4) | ((entry[2] >> 4) << 0)
    for entry in palette
]
palette = palette + (16 - len(palette)) * [0]

palbytes = bytearray()
for entry in palette:
    palbytes.append(entry & 0xFF)
    palbytes.append(entry >> 8)

# Generate bitmap RAM
bitmapram = bytearray()
for line in range(200):
    for x in range(0, 160, 2):
        pix1 = image.getpixel((x + 0, line))
        pix2 = image.getpixel((x + 1, line))
        bitmapram.append((pix1 << 4) | pix2)


bitmapram = bitmapram + palbytes
# bitmapram = bitmapram.ljust(16384, b"\x00")

args.output.write(bitmapram)
