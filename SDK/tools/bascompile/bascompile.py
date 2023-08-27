#!/usr/bin/env python3

import argparse
import sys
import struct
from lib.defs import Statements, Tokens, Operation, Variable, StringLiteral
from lib.tokenizer import tokenize
from lib.parser import parse

parser = argparse.ArgumentParser(description="Convert BASIC text file to executable")
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file")
args = parser.parse_args()

lines, variableNames = parse(args.input)


f = open(args.output, "wt")

# String descriptor: four bytes: Length, ignored, Text Address


# [M80] MATHPK FOR BASIC MCS 8080  GATES/ALLEN/DAVIDOFF
#
# [M80] EXTERNAL LOCATIONS USED BY THE MATH-PACKAGE
#
# [M80] THE FLOATING ACCUMULATOR
# (M80) FACLO:   [LOW ORDER OF MANTISSA (LO)]
# (M80) FACMO:   [MIDDLE ORDER OF MANTISSA (MO)]
# (M80) FACHO:   [HIGH ORDER OF MANTISSA (HO)]
# (M80) FAC:     [EXPONENT]
# (M80)          [TEMPORARY COMPLEMENT OF SIGN IN MSB]
# (M80] ARGLO:           [LOCATION OF SECOND ARGUMENT]
# (M80] ARG:
# (M80] FBUFFR:  BUFFER FOR FOUT
# [M80]
# [M80] THE FLOATING POINT FORMAT IS AS FOLLOWS:
# [M80]
# [M80] THE SIGN IS THE FIRST BIT OF THE MANTISSA
# [M80] THE MANTISSA IS 24 BITS LONG
# [M80] THE BINARY POINT IS TO THE LEFT OF THE MSB
# [M80] NUMBER = MANTISSA * 2 ^ EXPONENT
# [M80] THE MANTISSA IS POSITIVE, WITH A ONE ASSUMED TO BE WHERE THE SIGN BIT IS
# [M80] THE SIGN OF THE EXPONENT IS THE FIRST BIT OF THE EXPONENT
# [M80] THE EXPONENT IS STORED IN EXCESS 200 I.E. WITH A BIAS OF 200
# [M80] SO, THE EXPONENT IS A SIGNED 8-BIT NUMBER WITH 200 ADDED TO IT
# [M80] AN EXPONENT OF ZERO MEANS THE NUMBER IS ZERO, THE OTHER BYTES ARE IGNORED
# [M80] TO KEEP THE SAME NUMBER IN THE FAC WHILE SHIFTING:
# [M80]  TO SHIFT RIGHT, EXP:=EXP+1
# [M80]  TO SHIFT LEFT,  EXP:=EXP-1
# [M80]
# [M80] SO, IN MEMORY THE NUMBER LOOKS LIKE THIS:
# [M80]  [BITS 17-24 OF THE MANTISSA]
# [M80]  [BITS 9-16 OF THE MANTISSA]
# [M80]  [THE SIGN IN BIT 7, BITS 2-8 OF THE MANTISSA ARE IN BITS 6-0]
# [M80]  [THE EXPONENT AS A SIGNED NUMBER + 200]
# [M80] (REMEMBER THAT BIT 1 OF THE MANTISSA IS ALWAYS A ONE)
# [M80]
# [M80] ARITHMETIC ROUTINE CALLING CONVENTIONS:
# [M80]
# [M80] FOR ONE ARGUMENT FUNCTIONS:
# [M80]  THE ARGUMENT IS IN THE FAC, THE RESULT IS LEFT IN THE FAC
# [M80] FOR TWO ARGUMENT OPERATIONS:
# [M80]  THE FIRST ARGUMENT IS IN B,C,D,E I.E. THE "REGISTERS"
# [M80]  THE SECOND ARGUMENT IS IN THE FAC
# [M80]  THE RESULT IS LEFT IN THE FAC
# [M80]
# [M80] THE "S" ENTRY POINTS TO THE TWO ARGUMENT OPERATIONS HAVE (HL) POINTING TO
# [M80] THE FIRST ARGUMENT INSTEAD OF THE FIRST ARGUMENT BEING IN THE REGISTERS.
# [M80] MOVRM IS CALLED TO GET THE ARGUMENT IN THE REGISTERS.
# [M80] THE "T" ENTRY POINTS ASSUME THE FIRST ARGUMENT IS ON THE STACK.
# [M80] POPR IS USED TO GET THE ARGUMENT IN THE REGISTERS.
# [M80] NOTE: THE "T" ENTRY POINTS SHOULD ALWAYS BE JUMPED TO AND NEVER CALLED
# [M80] BECAUSE THE RETURN ADDRESS ON THE STACK WILL BE CONFUSED WITH THE NUMBER.
# [M80]
# [M80] ON THE STACK, THE TWO LO'S ARE PUSHED ON FIRST AND THEN THE HO AND SIGN.
# [M80] THIS IS DONE SO IF A NUMBER IS STORED IN MEMORY, IT CAN BE PUSHED ON THE
# [M80] STACK WITH TWO PUSHM'S.  THE LOWER BYTE OF EACH PART IS IN THE LOWER
# [M80] MEMORY ADDRESS SO WHEN THE NUMBER IS POPPED INTO THE REGISTERS, THE HIGHER
# [M80] ORDER BYTE WILL BE IN THE HIGHER ORDER REGISTER OF THE REGISTER PAIR, I.E.
# [M80] THE HIGHER ORDER BYTE WILL BE POPPED INTO B, D OR H.


# [M80] FACLO   equ     $38E4   ;[M80] LOW ORDER OF MANTISSA
# {M80} FACMO   equ     $38E5   ;[M80] MIDDLE ORDER OF MANTISSA
# [M80] FACHO   equ     $38E6   ;[M80] HIGH ORDER OF MANTISSA
# [M80] FAC     equ     $38E7   ;[M80] EXPONENT
# [M80] FBUFFR  equ     $38E8   ;[M80] BUFFER FOR FOUT  (14 bytes)
# [M65] RESHO   equ     $38F6   ;[M65] RESULT OF MULTIPLIER AND DIVIDER
# [M65] RESMO   equ     $38F7   ;;RESMO and RESLO are loaded into and stored from HL
# {M65} RESLO   equ     $38F8   ;

# FOUT $1680       convert number in FAC to string in FBUFFR


print(
    """
; RST
START:   equ $00     ; 0 Start/Reboot
SYNCHR:  equ $08     ; 1 SN Error if next character does not match
CHRGET:  equ $10     ; 2 Get Next Character
OUTCHR:  equ $18     ; 3 Output Character
COMPAR:  equ $20     ; 4 Compare HL with DE
FSIGN:   equ $28     ; 5 Get sign of Floating Point Argument
HOOKDO:  equ $30     ; 6 Extended BASIC Hook Dispatch
INTRPT:  equ $38     ; 7 (S3BASIC) Execute Interrupt Routine
    
; Floating point routines in system ROM
FADDH:   equ $1250      ; FAC = 0.5 + FAC
FADDS:   equ $1253      ; FAC = (hl) + FAC
FSUBS:   equ $1258      ; FAC = (hl) - FAC
FSUBT:   equ $125C      ; FAC = ARG_from_stack - FAC 
FSUB:    equ $125E      ; FAC = ARG - FAC
FADD:    equ $1261      ; FAC = ARG + FAC
ZERO:    equ $12C3      ; FAC = 0
ROUND:   equ $12F1      ; FAC = round(ARG)
NEGR:    equ $131C      ; ARG = -ARG
LOG:     equ $1385      ; FAC = log(ARG)
LOG2:    equ $1395      ; FAC = log2(ARG)
FMULTT:  equ $13C9      ; FAC = ARG_from_stack * FAC
FMULT:   equ $13CB      ; FAC = ARG * FAC
FDIVT:   equ $142D      ; FAC = ARG_from_stack / FAC
FDIV:    equ $142F      ; FAC = ARG / FAC
SGN:     equ $14F5      ; 
FLOAT:   equ $14F6      ;
FLOATR:  equ $14FB      ;
ABS:     equ $1509      ; FAC = abs(FAC)
NEG:     equ $150B      ; FAC = -FAC
PUSHF:   equ $1513      ; Push FAC to stack
MOVFM:   equ $1520      ; FAC = (hl)
MOVFR:   equ $1523      ; FAC = ARG
MOVRF:   equ $152E      ; ARG = FAC
MOVRM:   equ $1531      ; ARG = (hl)
MOVMF:   equ $153A      ; (hl) = FAC
FCOMP:   equ $155B      ; A=1 when ARG<FAC, A=0 when ARG==FAC, A=-1 when ARG>FAC
QINT:    equ $1586      ;
INT:     equ $15B1      ;
FIN:     equ $15E5      ;
FADDT:   equ $165C      ; FAC = ARG_from_stack + FAC 
FOUT:    equ $1680      ;
SQR:     equ $1775      ; FAC = sqrt(FAC)  (FAC = FAC ^ 0.5)
FPWRT:   equ $177E      ; FAC = ARG_from_stack ^ FAC
FPWR:    equ $1780      ; FAC = ARG ^ FAC
EXP:     equ $17CD      ; FAC = exp(FAC)
RND:     equ $1866      ; FAC = rnd(ARG)
COS:     equ $18D7      ; FAC = cos(FAC)
SIN:     equ $18DD      ; FAC = sin(FAC)
TAN:     equ $1970      ; FAC = tan(FAC)
ATN:     equ $1985      ; FAC = atan(FAC) - not implemented 8K BASIC


STROUT:  equ $0E9D
CRDO:    equ $19EA

FBUFFR:  equ $38E8

    org $38E1

    ; Header and BASIC stub
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp main

main:
    push hl

""",
    file=f,
)


# Info from https://community.embarcadero.com/index.php/article/technical-articles/162-programming/14799-converting-between-microsoft-binary-and-ieee-forma
def float_to_mbf32(value):
    ieee_val = struct.unpack("I", struct.pack("f", value))[0]
    mbf_exp = (((ieee_val >> 23) & 0xFF) + 2) & 0xFF
    sign = ieee_val >> 31
    mbf_val = (mbf_exp << 24) | (sign << 23) | (ieee_val & 0x7FFFFF)
    return mbf_val


# exit()


for line in lines:
    print(f"l{line[0]}:", file=f)

    for stmt in line[1]:
        print(f"    ; {stmt}", file=f)
        if stmt[0] == Statements.LET:
            varName = stmt[1].name
            expr = stmt[2]
            mbf_val = float_to_mbf32(expr)
            print(f"    ld   bc, ${mbf_val >> 16:04x}", file=f)
            print(f"    ld   de, ${mbf_val & 0xFFFF:04x}", file=f)
            print(f"    ld   (v{varName}+2), bc", file=f)
            print(f"    ld   (v{varName}), de", file=f)

        elif stmt[0] == Statements.GOTO:
            print(f"    jp   l{stmt[1]:.0f}", file=f)
        elif stmt[0] == Statements.END:
            print(f"    jp   end", file=f)
        elif stmt[0] == Statements.PRINT:
            for expr in stmt[1]:
                if isinstance(expr, float):
                    mbf_val = float_to_mbf32(expr)
                    print(f"    ld   bc, ${mbf_val >> 16:04x}", file=f)
                    print(f"    ld   de, ${mbf_val & 0xFFFF:04x}", file=f)
                    print(f"    call MOVFR", file=f)
                    print(f"    call FOUT", file=f)
                    print(f"    ld   hl, FBUFFR+1", file=f)
                    print(f"    call STROUT", file=f)
                elif isinstance(expr, Variable):
                    print(f"    ld   hl, v{expr.name}", file=f)
                    print(f"    call MOVFM", file=f)
                    print(f"    call FOUT", file=f)
                    print(f"    ld   hl, FBUFFR+1", file=f)
                    print(f"    call STROUT", file=f)
                else:
                    print(f"Unhandled statement {stmt}")

            print(f"    call CRDO", file=f)

        else:
            print(f"Unhandled statement {stmt}")

print(
    """
end:
    pop hl
    ret
""",
    file=f,
)

for name in variableNames:
    print(f"v{name}: defd 0", file=f)


print(
    """
    ; A valid CAQ file needs 15 zeros at the end of the file
    db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
""",
    file=f,
    end="",
)
