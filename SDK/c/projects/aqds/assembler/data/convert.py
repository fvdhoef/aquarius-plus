#!/usr/bin/env python3

with open("opcodes.lst") as f:
    lines = f.readlines()

total = 0

opcodes = set()

for line in lines:
    opcode_bytes = line[0:11].strip()
    opcode_text = line[11:].strip()
    opcodes.add(opcode_text.split(" ")[0])

directives = sorted(
    [
        a.upper()
        for a in [
            "org",
            "equ",
            "db",
            "defb",
            "ds",
            "defs",
            "dw",
            "defw",
            "include",
            "incbin",
            "phase",
            "dephase",
            "end",
            "assert",
        ]
    ]
)

opcodes = sorted(list(opcodes))

with open("../tokens.h", "wt") as f:
    print("#ifndef _TOKENS_H", file=f)
    print("#define _TOKENS_H", file=f)
    print(file=f)
    print("#include <stdint.h>", file=f)
    print(file=f)
    print("extern const uint8_t *keywords[];", file=f)
    print("extern const uint8_t num_keywords[];", file=f)
    print(file=f)
    print("enum {", file=f)

    idx = 0
    print(f"    TOK_UNKNOWN = {idx},", file=f)
    idx += 1
    print(file=f)
    print("    // Directives", file=f)
    print(f"    TOK_DIR_FIRST = {idx},", file=f)
    for val in directives:
        print(f"    TOK_{val.upper()} = {idx},", file=f)
        idx += 1
    print(f"    TOK_DIR_LAST = {idx-1},", file=f)
    print(file=f)
    print("    // Opcodes", file=f)
    print(f"    TOK_OPCODE_FIRST = {idx},", file=f)
    for val in opcodes:
        print(f"    TOK_{val.upper()} = {idx},", file=f)
        idx += 1
    print(f"    TOK_OPCODE_LAST = {idx-1},", file=f)
    print("};", file=f)
    print(file=f)
    print("#endif", file=f)

keywords = sorted(directives + opcodes)

with open("../tokens.c", "wt") as f:
    print('#include "tokens.h"', file=f)
    print(file=f)
    print("// clang-format off", file=f)

    for i in range(2, 8):
        print(f"static const uint8_t keywords{i}[] = {{", file=f)
        for keyword in keywords:
            if len(keyword) != i:
                continue

            chars = [f"'{ch}'," for ch in keyword.lower()]
            chars.append(f"TOK_{keyword.upper()},")

            print("    " + "".join(chars), file=f)

        print("};", file=f)

    print("// clang-format on", file=f)
    print(file=f)
    print("const uint8_t *keywords[] = {", file=f)
    for i in range(2, 8):
        print(f"    keywords{i},", file=f)
    print("};", file=f)
    print("const uint8_t num_keywords[] = {", file=f)
    for i in range(2, 8):
        print(f"    sizeof(keywords{i}) / {i+1},", file=f)
    print("};", file=f)
