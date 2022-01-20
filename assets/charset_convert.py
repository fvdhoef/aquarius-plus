#!/usr/bin/env python3
import PIL.Image

image = PIL.Image.open("aquarius charset.png")

with open("charset.c", "w") as f:
    print("unsigned char charset[256 * 8 * 8] = {", file=f)

    for k in range(256):
        for j in range(8):
            hpixels = [0] * 8

            for i in range(8):
                if image.getpixel(((k % 16) * 8 + i, (k // 16) * 8 + j))[0] > 0:
                    hpixels[i] = 1

            print("    " + ", ".join(f"{x}" for x in hpixels) + ",", file=f)
        print("", file=f)

    print("};", file=f)
