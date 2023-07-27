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

# Turn into black/white bitmap, if not already that way
image = image.convert('L')

# Generate bitmap RAM
bitmapram = bytearray()
for line in range(128):
    for column in range(16):
        myBits = [0] * 8
        myByte = 0
        for pixel in range(8):
            coordinate = x, y = (column % 16) * 8 + pixel, (column // 16) * 8 + line
            if image.getpixel(coordinate)[0] > 0:
                myBits[pixel] = 1
            myByte |= myBits[pixel]
            myByte <<= 1
        bitmapram.append(myByte)

bitmapram = bitmapram.ljust(2048, b"\x00")

args.output.write(bitmapram)
