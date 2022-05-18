#!/usr/bin/env python3
#-----------------------------------------------------------------------------
# Copyright (C) 2022 Frank van den Hoef
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice,
#    this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE
#-----------------------------------------------------------------------------
#
# Prerequisites: Python3 installed and executable defined in PATH
#       Caveats: Line numbers cannot be less than 0 or greater than 65529
#                Characters per line should be 72 or less
#                KILL and DEL commands resolve to same token code
#         Usage: python3 txt2bas.py progname.txt progname.bas
#
#-----------------------------------------------------------------------------

import argparse
import struct
import sys
import re
from xmlrpc.client import TRANSPORT_ERROR

parser = argparse.ArgumentParser(
    description="Convert text file to Aquarius BASIC .BAS file"
)
parser.add_argument("input", help="Input file", type=argparse.FileType("r"))
parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))

args = parser.parse_args()

tokens = {
    0x80: "END",
    0x81: "FOR",
    0x82: "NEXT",
    0x83: "DATA",
    0x84: "INPUT",
    0x85: "DIM",
    0x86: "READ",
    0x87: "LET",
    0x88: "GOTO",
    0x89: "RUN",
    0x8A: "IF",
    0x8B: "RESTORE",
    0x8C: "GOSUB",
    0x8D: "RETURN",
    0x8E: "REM",
    0x8F: "STOP",
    0x90: "ON",
    0x91: "LPRINT",
    0x92: "COPY",
    0x93: "DEF",
    0x94: "POKE",
    0x95: "PRINT",
    0x96: "CONT",
    0x97: "LIST",
    0x98: "LLIST",
    0x99: "CLEAR",
    0x9A: "CLOAD",
    0x9B: "CSAVE",
    0x9C: "PSET",
    0x9D: "PRESET",
    0x9E: "SOUND",
    0x9F: "NEW",
    0xA0: "TAB(",
    0xA1: "TO",
    0xA2: "FN",
    0xA3: "SPC(",
    0xA4: "INKEY$",
    0xA5: "THEN",
    0xA6: "NOT",
    0xA7: "STEP",
    0xA8: "+",
    0xA9: "-",
    0xAA: "*",
    0xAB: "/",
    0xAC: "^",
    0xAD: "AND",
    0xAE: "OR",
    0xAF: ">",
    0xB0: "=",
    0xB1: "<",
    0xB2: "SGN",
    0xB3: "INT",
    0xB4: "ABS",
    0xB5: "USR",
    0xB6: "FRE",
    0xB7: "LPOS",
    0xB8: "POS",
    0xB9: "SQR",
    0xBA: "RND",
    0xBB: "LOG",
    0xBC: "EXP",
    0xBD: "COS",
    0xBE: "SIN",
    0xBF: "TAN",
    0xC0: "ATN",
    0xC1: "PEEK",
    0xC2: "LEN",
    0xC3: "STR$",
    0xC4: "VAL",
    0xC5: "ASC",
    0xC6: "CHR$",
    0xC7: "LEFT$",
    0xC8: "RIGHT$",
    0xC9: "MID$",
    0xCA: "POINT",
    # Start of supplemental USB BASIC commands
    # 0xD4: "<reserved>",
    0xD5: "CLS",
    0xD6: "LOCATE",
    0xD7: "OUT",
    0xD8: "PSG",
    # 0xD9: "<reserved>",
    0xDA: "CALL",
    0xDB: "LOAD",
    0xDC: "SAVE",
    0xDD: "DIR",
    # 0xDE: "<reserved>",
    0xDF: "DEL",
    0xE0: "CD",
    0xE1: "IN",
    0xE2: "JOY",
    0xE3: "HEX$",
}


def error(idx, message):
    print(f"{args.input.name}:{idx+1} {message}", file=sys.stderr)
    exit(1)


# Write header
args.output.write(
    12 * b"\xFF"
    + b"\x00"
    + (args.input.name.upper() + "      ")[0:6].encode()
    + 12 * b"\xFF"
    + b"\x00"
)


last_linenr = -1
addr = 0x3903

# Tokenize lines
for idx, line in enumerate(args.input.readlines()):
    line = line.strip()
    if not line:
        continue

    result = re.search("^([0-9]+)(.*)$", line)
    if result == None:
        error(idx, "Syntax error")

    linenr = int(result.group(1))
    line = result.group(2).strip()

    if linenr < last_linenr or linenr > 65000:
        error(idx, "Invalid line number")

    buf = bytearray()
    buf += struct.pack("<H", linenr)

    in_str = False
    in_rem = False

    while len(line) > 0:
        upper = line.upper()

        if line[0] == '"':
            in_str = not in_str

        if not (in_str or in_rem) and line[0] != " ":
            found = False
            for (token, keyword) in tokens.items():
                if upper.startswith(keyword):
                    buf.append(token)
                    line = line[len(keyword) :]
                    found = True

                    if keyword == "REM":
                        in_rem = True

                    break

            if found:
                continue

        buf.append(line[0].encode()[0])
        line = line[1:]

    buf.append(0)

    buf = struct.pack("<H", addr + len(buf)) + buf
    addr += len(buf)
    last_linenr = linenr

    args.output.write(buf)

args.output.write(struct.pack("<H", 0))
