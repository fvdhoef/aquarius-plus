#!/usr/bin/env python3
import xml.etree.ElementTree as ET
import PIL.Image

tree = ET.parse("tiledemo.tmx")
mapdata = tree.getroot().find("layer").find("data").text
mapdata = [int(entry.strip()) - 1 + 128 for entry in mapdata.split(",")]

tilemap = bytearray()
for entry in mapdata:
    tilemap.append(entry & 0xFF)
    tilemap.append(entry >> 8)

image = PIL.Image.open("s1sms_ghz1.png")
image = image.convert(mode="P")

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

patterns = bytearray()
for row in range(16):
    for column in range(16):
        for j in range(8):
            for i in range(4):
                idx0 = image.getpixel((column * 8 + i * 2 + 0, row * 8 + j))
                idx1 = image.getpixel((column * 8 + i * 2 + 1, row * 8 + j))
                patterns.append(idx0 << 4 | idx1)

with open("tiledata.bin", "wb") as f:
    f.write(tilemap)
    f.write(patterns)
    f.write(bytearray(palbytes))
