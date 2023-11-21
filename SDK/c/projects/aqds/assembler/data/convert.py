#!/usr/bin/env python3

with open("opcodes.lst") as f:
    lines = f.readlines()

total = 0

for line in lines:
    opcode_bytes = line[0:11].strip()
    opcode_text = line[11:].strip()

    total += len(opcode_bytes) / 2
    total += len(opcode_text)

    # if len(opcode_bytes) == 2:

        # bla.append(opcode_bytes)
        # bla.append(opcode_text)


        # print(f'{{"{opcode_text}", 0x{opcode_bytes}}},')
        # print(f"{opcode_bytes} {opcode_text}")

        # print(f'{"{opcode_text}", opcode_bytes}')

print(total)
