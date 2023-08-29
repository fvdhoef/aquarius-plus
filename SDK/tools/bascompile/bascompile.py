#!/usr/bin/env python3

import argparse
import sys
import struct
from lib.defs import Statements, Tokens, Operation, Variable, StringLiteral
from lib.tokenizer import tokenize
from lib.parser import parse
from enum import Enum

parser = argparse.ArgumentParser(description="Convert BASIC text file to executable")
parser.add_argument("input", help="Input file")
parser.add_argument("output", help="Output file")
args = parser.parse_args()

lines, variableNames = parse(args.input)
forLoopNames = set()


f = open(args.output, "wt")

# String descriptor: four bytes: Length, ignored, Text Address


# Info from https://community.embarcadero.com/index.php/article/technical-articles/162-programming/14799-converting-between-microsoft-binary-and-ieee-forma
def float_to_mbf32(value):
    ieee_val = struct.unpack("I", struct.pack("f", value))[0]
    if ieee_val == 0:
        return 0

    mbf_exp = (((ieee_val >> 23) & 0xFF) + 2) & 0xFF
    sign = ieee_val >> 31
    mbf_val = (mbf_exp << 24) | (sign << 23) | (ieee_val & 0x7FFFFF)
    return mbf_val


lbl_idx = 1


def get_lbl():
    global lbl_idx
    tmp_lbl = f"l{lbl_idx}"
    lbl_idx += 1
    return tmp_lbl


def emit_lbl(name):
    print(f"{name}:", file=f)


def emit(instr):
    print(f"    {instr}", file=f)


def emit_hdr():
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
SGN:     equ $14F5      ; FAC = ARG < 0 ? -1 : ARG > 0 ? 1 : 0
FLOAT:   equ $14F6      ; FAC = float(A)
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
INT:     equ $15B1      ; FAC = int(FAC)
FIN:     equ $15E5      ;
FADDT:   equ $165C      ; FAC = ARG_from_stack + FAC 
FOUT:    equ $1680      ; Convert number in FAC to string in FBUFFR+1
SQR:     equ $1775      ; FAC = sqrt(FAC)  (FAC = FAC ^ 0.5)
FPWRT:   equ $177E      ; FAC = ARG_from_stack ^ FAC
FPWR:    equ $1780      ; FAC = ARG ^ FAC
EXP:     equ $17CD      ; FAC = exp(FAC)
RND:     equ $1866      ; FAC = rnd(ARG)
COS:     equ $18D7      ; FAC = cos(FAC)
SIN:     equ $18DD      ; FAC = sin(FAC)
TAN:     equ $1970      ; FAC = tan(FAC)
ATN:     equ $1985      ; FAC = atan(FAC) - not implemented 8K BASIC

FRCINT:  equ $0682      ; de = (int)FAC
GIVINT:  equ $0B21      ; FAC = float(MSB:a LSB:c)
FLOATB:  equ $0B22      ; FAC = float(MSB:a LSB:b)
FLOATD:  equ $0B23      ; FAC = float(MSB:a LSB:d)
STROUT:  equ $0E9D
CRDO:    equ $19EA

FACLO:   equ $38E4      ; FAC low order of mantissa
FACMO:   equ $38E5      ; FAC middle order of mantissa
FACHO:   equ $38E6      ; FAC high order of mantissa
FAC:     equ $38E7      ; FAC exponent
FBUFFR:  equ $38E8

    org $38E1

    ; Header and BASIC stub
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db "AQPLUS"
    db $FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$FF,$00
    db $0E,$39,$0A,$00,$DA,"14608",':',$80,$00,$00,$00
    jp entry
    """,
        file=f,
    )


def assign_float_to_arg(val):
    mbf_val = float_to_mbf32(val)
    emit(f"ld   bc, ${mbf_val >> 16:04x}")
    emit(f"ld   de, ${mbf_val & 0xFFFF:04x}")


def assign_float_to_fac(val):
    assign_float_to_arg(val)
    emit(f"call MOVFR")


def save_arg(varName):
    emit(f"ld   (var{varName}+2), bc")
    emit(f"ld   (var{varName}), de")


def save_fac(varName):
    emit(f"ld   hl, var{varName}")
    emit(f"call MOVMF")


def load_fac(varName):
    emit(f"ld   hl, var{varName}")
    emit(f"call MOVFM")


def load_arg(varName):
    emit(f"ld   hl, var{varName}")
    emit(f"call MOVRM")


def print_fac():
    # This destructs the value of FAC
    emit(f"call FOUT")
    emit(f"ld   hl, FBUFFR+1")
    emit(f"call STROUT")


def push_fac():
    emit(f"call PUSHF")


def pop_arg():
    emit(f"pop  bc")
    emit(f"pop  de")


def emit_unary_op(expr, func):
    emit_expr(expr[1])
    emit(f"call {func}")


def emit_binary_op(expr, func):
    emit_expr(expr[1])
    push_fac()
    emit_expr(expr[2])
    pop_arg()
    emit(f"call {func}")


def emit_compare(expr):
    emit_expr(expr[1])
    push_fac()
    emit_expr(expr[2])
    pop_arg()
    emit(f"call FCOMP")


def emit_expr(expr):
    if isinstance(expr, float):
        assign_float_to_fac(expr)

    elif isinstance(expr, Variable):
        load_fac(expr.name)

    elif isinstance(expr, tuple):
        if expr[0] == Operation.NEGATE:
            emit_unary_op(expr, "NEG")
        elif expr[0] == Operation.MULT:
            emit_binary_op(expr, "FMULT")
        elif expr[0] == Operation.DIV:
            emit_binary_op(expr, "FDIV")
        elif expr[0] == Operation.ADD:
            emit_binary_op(expr, "FADD")
        elif expr[0] == Operation.SUB:
            emit_binary_op(expr, "FSUB")
        elif expr[0] == Operation.POW:
            emit_binary_op(expr, "FPWR")
        elif expr[0] == Operation.SIN:
            emit_unary_op(expr, "SIN")
        elif expr[0] == Operation.COS:
            emit_unary_op(expr, "COS")
        elif expr[0] == Operation.TAN:
            emit_unary_op(expr, "TAN")
        elif expr[0] == Operation.ATN:
            emit_unary_op(expr, "ATN")
        elif expr[0] == Operation.LOG:
            emit_unary_op(expr, "LOG")
        elif expr[0] == Operation.EXP:
            emit_unary_op(expr, "EXP")
        elif expr[0] == Operation.RND:
            emit_unary_op(expr, "RND")
        elif expr[0] == Operation.SQR:
            emit_unary_op(expr, "SQR")
        elif expr[0] == Operation.ABS:
            emit_unary_op(expr, "ABS")
        elif expr[0] == Operation.SGN:
            emit_unary_op(expr, "SGN")
        elif expr[0] == Operation.INT:
            emit_unary_op(expr, "INT")
        elif expr[0] == Operation.NOT:
            emit_expr(expr[1])
            emit(f"call FRCINT")
            # Should we check Z flag here for out of range?
            emit(f"ld   a, $FF")
            emit(f"xor  e")
            emit(f"ld   b, a")
            emit(f"ld   a, $FF")
            emit(f"xor  d")
            emit(f"call FLOATB")

        elif expr[0] == Operation.AND:
            emit_expr(expr[1])
            emit(f"call FRCINT")
            emit(f"push de")
            emit_expr(expr[2])
            emit(f"call FRCINT")
            emit(f"pop  bc")
            emit(f"ld   a,c")
            emit(f"and  e")
            emit(f"ld   c,a")
            emit(f"ld   a,b")
            emit(f"and  d")
            emit(f"call GIVINT")

        elif expr[0] == Operation.OR:
            emit_expr(expr[1])
            emit(f"call FRCINT")
            emit(f"push de")
            emit_expr(expr[2])
            emit(f"call FRCINT")
            emit(f"pop  bc")
            emit(f"ld   a,c")
            emit(f"or   e")
            emit(f"ld   c,a")
            emit(f"ld   a,b")
            emit(f"or   d")
            emit(f"call GIVINT")

        elif expr[0] == Operation.EQ:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   0")
            emit(f"ld   a, 0")
            emit(f"jp   NZ, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        elif expr[0] == Operation.NE:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   0")
            emit(f"ld   a, 0")
            emit(f"jp   Z, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        elif expr[0] == Operation.LT:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   1")
            emit(f"ld   a, 0")
            emit(f"jp   NZ, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        elif expr[0] == Operation.LE:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   -1")
            emit(f"ld   a, 0")
            emit(f"jp   Z, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        elif expr[0] == Operation.GE:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   1")
            emit(f"ld   a, 0")
            emit(f"jp   Z, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        elif expr[0] == Operation.GT:
            emit_compare(expr)
            lbl = get_lbl()
            emit(f"cp   -1")
            emit(f"ld   a, 0")
            emit(f"jp   NZ, {lbl}")
            emit(f"ld   a, -1")
            emit_lbl(lbl)
            emit(f"call FLOAT")

        else:
            print(f"Unhandled expression {expr}")
            exit(1)
    else:
        print(f"Unhandled expression {expr}")
        exit(1)


def emit_statement(stmt):
    put_nxt_line_lbl = False
    emit(f"; {stmt}")

    if stmt[0] == Statements.LET:
        varName = stmt[1].name
        expr = stmt[2]
        emit_expr(expr)
        save_fac(varName)

    elif stmt[0] == Statements.GOTO:
        emit(f"jp   line{stmt[1]:.0f}")

    elif stmt[0] == Statements.END:
        emit(f"jp   exit")

    elif stmt[0] == Statements.PRINT:
        for expr in stmt[1]:
            emit_expr(expr)
            print_fac()
        emit(f"call CRDO")

    elif stmt[0] == Statements.IF:
        emit_expr(stmt[1])
        emit(f"ld   a,(FAC)")
        emit(f"or   a")
        emit(f"jp   z, .next_line")
        emit_statement(stmt[2])
        put_nxt_line_lbl = True

    elif stmt[0] == Statements.FOR:
        forLoopNames.add(stmt[1].name)
        emit_expr(stmt[2])
        save_fac(stmt[1].name)
        emit_expr(stmt[3])
        emit(f"ld   hl, forTo{stmt[1].name}")
        emit(f"call MOVRM")
        emit_expr(stmt[4])
        emit(f"ld   hl, forStp{stmt[1].name}")
        emit(f"call MOVRM")
        lbl = get_lbl()
        emit(f"ld   bc, {lbl}")
        emit(f"ld   (forPtr{stmt[1].name}), bc")
        emit_lbl(lbl)

    elif stmt[0] == Statements.NEXT:
        pass

    elif stmt[0] == Statements.POKE:
        emit_expr(stmt[1])
        emit(f"call FRCINT")
        emit(f"push de")
        emit_expr(stmt[2])
        emit(f"call FRCINT")
        emit(f"ld   a,e")
        emit(f"pop  de")
        emit(f"ld   (de),a")

    else:
        print(f"Unhandled statement {stmt}")
        exit(1)

    return put_nxt_line_lbl


emit_hdr()
emit_lbl("entry")
emit("push hl")

for line in lines:
    emit_lbl(f"line{line[0]}")

    put_nxt_line_lbl = False
    for stmt in line[1]:
        if emit_statement(stmt):
            put_nxt_line_lbl = True

    if put_nxt_line_lbl:
        emit_lbl(".next_line")

emit_lbl("exit")
emit("pop  hl")
emit("ret")
emit("")

for name in sorted(list(variableNames)):
    print(f"var{name}: defd 0", file=f)

for name in sorted(list(forLoopNames)):
    print(f"forTo{name}: defd 0", file=f)
    print(f"forStp{name}: defd 0", file=f)
    print(f"forPtr{name}: defd 0", file=f)


print(
    """
    ; A valid CAQ file needs 15 zeros at the end of the file
    db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
""",
    file=f,
    end="",
)
