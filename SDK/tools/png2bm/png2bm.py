#!/usr/bin/env python3
import argparse
import PIL.Image

parser = argparse.ArgumentParser(
    description="Convert image file to Aq+ bitmap file"
)
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))
args = parser.parse_args()

image = PIL.Image.open(args.input)

if image.width != 320 or image.height != 200:
    print("Image must be 320x200")
    exit(1)

image = image.convert(
    mode="P", palette=PIL.Image.ADAPTIVE, dither=PIL.Image.FLOYDSTEINBERG
)

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

# Generate color RAM
colorram_data = []
for row in range(25):
    for column in range(40):
        colors = set()

        for y in range(8):
            for x in range(8):
                colors.add(image.getpixel((column * 8 + x, row * 8 + y)))

        colors = list(colors)
        if len(colors) == 1:
            colors = [colors[0], colors[0]]
        if len(colors) != 2:
            print(f"More than 2 colors ({len(colors)}) in row {row} column {column}")

        colorram_data.append(colors)

colorram = bytearray([(colors[0] << 4 | colors[1]) for colors in colorram_data])

# Generate bitmap RAM
bitmapram = bytearray()
for line in range(200):
    for column in range(40):
        colors = colorram_data[line // 8 * 40 + column]

        bm = 0
        for i in range(8):
            idx = colors.index(image.getpixel((column * 8 + i, line)))
            if idx == 0:
                bm |= 1 << (7 - i)

        bitmapram.append(bm)

bitmapram = bitmapram + palbytes
bitmapram = bitmapram.ljust(8192, b"\x00")

args.output.write(bitmapram)
args.output.write(colorram)
