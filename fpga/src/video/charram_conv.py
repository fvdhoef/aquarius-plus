#!/usr/bin/env python3

with open("../../../assets/AquariusCharacterSet.bin", "rb") as f:
    fontdata = f.read()


def chunks(lst, n):
    for i in range(0, len(lst), n):
        yield lst[i : i + n]


entries = [f"ram[{i:4d}] = 8'h{fontdata[i]:02X};" for i in range(len(fontdata))]

print("    initial begin")
for chunk in chunks(entries, 4):
    print(f"        {' '.join(chunk)}")
print("    end")
