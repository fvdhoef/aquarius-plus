#!/usr/bin/env python3
import PIL.Image

image = PIL.Image.open("font.png")

with open("font.asm", "w") as f:
    print("font_start:", file=f)

    for k in range(6 * 16 - 1):
        for j in range(8):
            hpixels = [0] * 8

            for i in range(8):
                if image.getpixel(((k % 16) * 8 + i, (k // 16) * 8 + j))[0] > 0:
                    hpixels[i] = 1

            bits = "".join(f"{x}" for x in hpixels) + "b"
            zero = "00000000b"

            print(f"    defb {bits}, {bits}, {bits}, {zero}", file=f)
        print("", file=f)

    print("font_end:", file=f)
