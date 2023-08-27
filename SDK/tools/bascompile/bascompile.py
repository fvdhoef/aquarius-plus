#!/usr/bin/env python3

import argparse
import sys
from lib.defs import Statements, Tokens, Operation, Variable, StringLiteral
from lib.tokenizer import tokenize
from lib.parser import parse

parser = argparse.ArgumentParser(description="Convert BASIC text file to executable")
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file")
args = parser.parse_args()

lines = parse(args.input)


f = open(args.output, "wt")

print(
    """
    org $38E1

    ; Header and BASIC stub
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp main

main:
""",
    file=f,
)


for line in lines:
    print(f"l{line[0]}:", file=f)

    for stmt in line[1]:
        print(f"    ; {stmt}", file=f)

        if stmt[0] == Statements.GOTO:
            print(f"    ; jp l{stmt[1]:.0f}", file=f)
        elif stmt[0] == Statements.END:
            print(f"    ret", file=f)

        else:
            print(f"Unhandled statement {stmt}")

print(
    """
    ; A valid CAQ file needs 15 zeros at the end of the file
    db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
""",
    file=f,
    end="",
)
