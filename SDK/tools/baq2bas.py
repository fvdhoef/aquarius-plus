#!/usr/bin/env python3
#-----------------------------------------------------------------------------
# Copyright (C) 2022 Frank van den Hoef
# Modifications (C) 2023 Curtis F Kaylor
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
#       Caveats: Byte code for line number cannot be greater than 0xFFF9
#                KILL and DEL tokens resolve to DEL command (your text KILL command will become a DEL command)
#         Usage: python3 bas2txt.py progname.bas progname.txt
#
#-----------------------------------------------------------------------------

import argparse
import struct
import sys

parser = argparse.ArgumentParser(
    description="Convert Aquarius BASIC .BAS file to text file"
)
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file", type=argparse.FileType("w"))

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
    # Start of plusBASIC keywords
    0xCB: "XOR",
    0xCC: "PUT",
    0xCD: "GET",
    0xCE: "DRAW",
   #0xCF:
    0xD0: "LINE",
    0xD1: "SWAP",
    0xD2: "DOKE",
    0xD3: "TIME",
    0xD4: "EDIT",
    0xD5: "CLS",
    0xD6: "LOCATE",
    0xD7: "OUT",
    0xD8: "PSG",
    0xD9: "MOUSE",
    0xDA: "CALL",
    0xDB: "LOAD",
    0xDC: "SAVE",
    0xDD: "DIR",
    0xDE: "MKDIR",
    0xDF: "DEL",
    0xE0: "CD",
    0xE1: "IN",
    0xE2: "JOY",
    0xE3: "HEX$",
    0xE4: "RENAME",
    0xE5: "DATE", 
    0xE6: "DEC",
    0xE7: "MOD",
    0xE8: "DEEK",
    0xE9: "ERR",
    0xEA: "STRING",
    0xEB: "BIT",
   #0xEC:
    0xED: "EVAL",
    0xEE: "PAUSE",
    0xEF: "ELSE",
    0xF0: "TILE",
    0xF1: "RGB",
    0xF2: "MAP",
    0xF3: "FILE",
    0xF4: "RESUME",
    0xF5: "COL",
    0xF6: "SCREEN",
    0xF7: "SET",
    0xF8: "WRITE",
    0xF9: "USE",
    0xFA: "OPEN",
    0xFB: "CLOSE"
   #0xFC:
   #0xFD:
}

xprefix = 0xFE
xtokens = {
    0x80: "ATTR",        
    0x81: "PALETTE",          
    0x82: "OFF",               
   #0x83 Unused (DATA) 
    0x84: "SPRITE",      
    0x85: "CHR",              
    0x86: "KEY",              
    0x87: "DEX",               
    0x88: "FAST",               
    0x89: "TEXT",               
    0x8A: "ARGS",               
    0x8B: "SAMPLE",               
    0x8C: "TRACK",
    0x8D: "PIXEL",
   #0x8E Unused (REM)
    0x8F: "NAME",
    0x90: "RESET",
    0x91: "EXT",
    0x92: "VER",
    0x93: "FILL",
    0x94: "COMPARE",
    0x95: "PLAY",
    0x96: "APPEND",
    0x97: "TRIM",
    0x98: "STASH",         
    0x99: "TRO",                       
    0x9A: "BREAK",                     
    0x9B: "LOOP",                     
    0x9C: "STR",          
    0x9D: "VAR",          
    0x9E: "ERASE",
    0x9F: "SPLIT",
    0xA0: "PAD",
    0xA1: "WORD",           
    0xA2: "CLIP",           
    0xA3: "PTR",       
    0xA4: "STATUS",
    0xA5: "BYTE",
    0xA6: "CAQ",
    0xA7: "MEM",
    0xA8: "JOIN",
    0xA9: "WAIT",
    0xAA: "CUR",
    0xAB: "HEX",
    0xAC: "BIN",
    0xAD: "MIN",
    0xAE: "MAX",
    0xAF: "UPR",
    0xB0: "LWR",
    0xB1: "SPEED",
    0xB2: "LONG",
    0xB3: "FLOAT",
    0xB4: "PATH"
    0xB4: "PATH",
    0xB5: "DUMP",
    0xB6: "BORDER",
    0xB7: "CHECK"
}

with open(args.input, "rb") as f:
    data = f.read()

    def check_header():
        for i in range(0, 12):
            if data[i] != 0xFF:
                return False
        if data[12] != 0:
                return False
        for i in range(19, 31):
            if data[i] != 0xFF:
                return False
        if data[31] != 0:
            return False

        print(f"Embedded filename: {data[13:19].decode()}")
        return True

    if data[0] == 0x00:
        idx = 1
    else:
        if not check_header():
            print("Incorrect format")
            exit(1)
        idx = 32

    while True:
        (nextline) = struct.unpack("<H", data[idx : idx + 2])[0]
        idx += 2
        if nextline == 0:
            break

        (linenr) = struct.unpack("<H", data[idx : idx + 2])[0]
        idx += 2
        linedata = bytearray()
        while data[idx] != 0:
            ch = data[idx]
            if ch & 0x80 == 0:
                linedata.append(data[idx])
            else:
                if ch == xprefix:
                    idx += 1
                    ch = data[idx]
                    tokenVal = xtokens.get(ch)
                    tokenType = "extended "
                else:
                    tokenVal = tokens.get(ch)
                    tokenType = ""
                if tokenVal == None:
                    print(f"Unknown "+tokenType+"token: 0x{ch:02X}")
                linedata += tokenVal.encode()
            idx += 1
        idx += 1

        print(f"{linenr} {linedata.decode()}", file=args.output)
