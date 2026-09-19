#!/usr/bin/env python3

import PIL.Image


f = open("tiledata.s", "wt")

image = PIL.Image.open("tetris.png")
assert image.mode == "P"
assert image.width == 128
assert image.height == 128

palette = image.getpalette()
palette = [palette[3 * n : 3 * n + 3] for n in range(256)][0 : len(image.getcolors())]
assert len(palette) <= 16

palette = [
    ((entry[0] >> 4) << 8) | ((entry[1] >> 4) << 4) | ((entry[2] >> 4) << 0)
    for entry in palette
]
palette = palette + (16 - len(palette)) * [0]

palbytes = bytearray()
for entry in palette:
    palbytes.append(entry & 0xFF)
    palbytes.append(entry >> 8)

print(f"    .area _CODE", file=f)
print(f"_tile_palette::", file=f)
for i in range(0, 32, 8):
    tmp = ", ".join([f"0x{val:02X}" for val in palbytes[i : i + 8]])
    print(f"    .db {tmp}", file=f)

tiledata = []

for tileidx in range(112):
    for j in range(8):
        y = (tileidx // 16) * 8 + j

        for i in range(0, 8, 2):
            x = (tileidx % 16) * 8 + i

            colidx1 = image.getpixel((x + 0, y))
            colidx2 = image.getpixel((x + 1, y))
            assert colidx1 <= 15 and colidx2 <= 15

            val = (colidx1 << 4) | colidx2
            tiledata.append(val)


print(file=f)
print(f"_tile_data::", file=f)

for i in range(0, len(tiledata), 8):
    tmp = ", ".join([f"0x{val:02X}" for val in tiledata[i : i + 8]])
    print(f"    .db {tmp}", file=f)

print(f"_tile_data_end::", file=f)

# print(tileidx, x, y, f"{val:02x}")

# print(image.getpixel((1, 1)))
