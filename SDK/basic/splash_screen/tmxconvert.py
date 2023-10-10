#!/usr/bin/env python3
from sys import stderr
import xml.etree.ElementTree as ET
import PIL.Image
import argparse

parser = argparse.ArgumentParser(
    description="Convert Tiled .tmx file and .png file to Aquarius+ VRAM format"
)
parser.add_argument(
    "tilemap",
    metavar="tilemap.tmx",
    help="Tiled .tmx file (tile layer should be 64x32 tiles)",
)
parser.add_argument("output", metavar="result.bin", help="Resulting VRAM binary image")
args = parser.parse_args()


tree = ET.parse(args.tilemap)
root = tree.getroot()

tileset = root.find("tileset")
firstgid = int(tileset.get("firstgid"))
image = tileset.find("image")
tileset_img = image.get("source")

layer = root.find("layer")
if int(layer.get("width")) != 64 or int(layer.get("height")) != 32:
    print("Tile layer should be 64x32 tiles.", file=stderr)
    exit(1)


mapdata = layer.find("data").text
mapdata = [int(entry.strip()) - firstgid + 128 for entry in mapdata.split(",")]

tilemap = bytearray()
for entry in mapdata:
    tilemap.append(entry & 0xFF)
    tilemap.append(entry >> 8)

image = PIL.Image.open(tileset_img)
if image.size[0] != 128 or image.size[1] != 192:
    print("Tileset is incorrect size, should be 128x192.", file=stderr)
    exit(1)

image = image.convert(mode="P")

palette = image.getpalette()
palette = [palette[3 * n : 3 * n + 3] for n in range(256)][0 : len(image.getcolors())]
if len(palette) > 16:
    print("Too many colors in tileset", file=stderr)
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

patterns = bytearray()
for row in range(24):
    for column in range(16):
        for j in range(8):
            for i in range(4):
                idx0 = image.getpixel((column * 8 + i * 2 + 0, row * 8 + j))
                idx1 = image.getpixel((column * 8 + i * 2 + 1, row * 8 + j))
                patterns.append(idx0 << 4 | idx1)

with open(args.output, "wb") as f:
    f.write(tilemap)
    f.write(patterns)
    f.write(bytearray(palbytes))
