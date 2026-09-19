#!/usr/bin/env python3

with open("tmp.txt") as f:
    lines = f.readlines()


for line in lines:
    line = line.strip()

    values = []
    for group in zip(*(iter(line),) * 8):
        nibbles = [int(x, 16) for x in group]

        val = 0
        for i in range(8):
            val |= nibbles[i] << (i * 4)

        values.append(val)

    print(" ".join([f"0x{val:08x}," for val in values]))

# print(lines)
