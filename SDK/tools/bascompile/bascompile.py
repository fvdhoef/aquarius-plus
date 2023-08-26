#!/usr/bin/env python3

import argparse
import struct
import sys
import re
from enum import Enum, auto

parser = argparse.ArgumentParser(description="Convert BASIC text file to executable")
parser.add_argument("input", help="Input file", type=argparse.FileType("r"))
# parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))
args = parser.parse_args()


def error(idx, message):
    print(f"{args.input.name}:{idx+1} {message}", file=sys.stderr)
    exit(1)


Statements = Enum(
    "Statements",
    [
        "CLEAR",
        "DIM",
        "END",
        "FOR",
        "GOTO",
        "IF",
        "INPUT",
        "LET",
        "NEXT",
        "ON",
        "POKE",
        "PRINT",
        #
        # Data
        "DATA",
        "READ",
        "RESTORE",
        #
        # Subroutines
        "GOSUB",
        "RETURN",
        #
        # Unsupported for now
        # "PSET",
        # "PRESET",
        # "RUN",
        # "SOUND",
        #
        # Extended commands
        "CALL",
        "CD",
        "CLS",
        "DEL",
        "DIR",
        "LOAD",
        "LOCATE",
        "OUT",
        "PSG",
        "SAVE",
    ],
)


def StatementsRepr(self):
    return self.name


Statements.__repr__ = StatementsRepr

Tokens = Enum(
    "Tokens",
    [
        "<>",
        "<=",
        ">=",
        "TAB(",
        "TO",
        "SPC(",
        "INKEY$",
        "THEN",
        "NOT",
        "STEP",
        "AND",
        "OR",
        "SGN(",
        "INT(",
        "ABS(",
        "USR(",
        "FRE(",
        "LPOS(",
        "POS(",
        "SQR(",
        "RND(",
        "LOG(",
        "EXP(",
        "COS(",
        "SIN(",
        "TAN(",
        "ATN(",
        "PEEK(",
        "LEN(",
        "STR$(",
        "VAL(",
        "ASC(",
        "CHR$(",
        "LEFT$(",
        "RIGHT$(",
        "MID$(",
        "POINT(",
        "IN(",
        "JOY(",
        "HEX$(",
    ],
)


class Variable:
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return f"<Var:{self.name}>"


class StringLiteral:
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return f"<String:'{self.name}'>"


def tokenize(lines):
    lastLineNr = -1
    programLines = []

    # Tokenize lines
    for idx, line in enumerate(lines):
        line = line.strip()
        if not line:
            continue

        # Get line number
        m = re.match("[0-9]+", line)
        if m == None:
            error(idx, "Syntax error")
        lineNr = int(m.group())
        line = line[m.end() :]

        if lineNr < lastLineNr or lineNr > 65000:
            error(idx, "Invalid line number")

        subLines = []
        subLine = []

        while len(line) > 0:
            # Skip whitespace
            if line[0] == " ":
                line = line[1:]
                continue

            # New sub-line?
            if line[0] == ":":
                if subLine:
                    subLines.append(subLine)
                    subLine = []
                line = line[1:]
                continue

            # String?
            if line[0] == '"':
                endIdx = line.find('"', 1)
                if endIdx < 0:
                    str = line[1:]
                    line = ""
                else:
                    str = line[1:endIdx]
                    line = line[endIdx + 1 :]

                subLine.append(StringLiteral(str))
                continue

            # Number?
            m = re.match("[0-9 ]*(?:\.[0-9 ]*)?(?:[eE][ ]*[-+]?[0-9 ]*)?", line)
            if m and m.group():
                valStr = m.group().replace(" ", "")
                if len(valStr) > 0 and valStr[0].upper() != "E":
                    line = line[m.end() :]
                    if valStr[0] == ".":
                        valStr = "0" + valStr
                    if valStr[-1].upper() == "E":
                        valStr += "0"
                    subLine.append(float(valStr))
                    continue

            # Check for statement
            found = False
            upper = line.upper()
            if upper.startswith("REM"):
                # Remark, skip rest of line
                line = ""
                continue
            for stmt in Statements:
                if upper.startswith(stmt.name):
                    subLine.append(stmt)
                    line = line[len(stmt.name) :]
                    found = True
                    break
            if found:
                continue

            # Check for keyword
            found = False
            upper = line.upper()
            for token in Tokens:
                if upper.startswith(token.name):
                    subLine.append(token)
                    line = line[len(token.name) :]
                    found = True
                    break
            if found:
                continue

            # Variable name?
            m = re.match("[A-Za-z]+[A-Za-z0-9]*\$?", line)
            if m and m.group():
                subLine.append(Variable(m.group().upper()))
                line = line[m.end() :]
                continue

            if line:
                subLine.append(line[0])
                line = line[1:]

        if subLine:
            subLines.append(subLine)

        programLines.append({"lineNr": lineNr, "subLines": subLines})

    return programLines


class Operation(Enum):
    NEGATE = auto()
    MULT = auto()
    DIV = auto()
    ADD = auto()
    SUB = auto()
    POW = auto()
    EQ = auto()
    NE = auto()
    LT = auto()
    LE = auto()
    GT = auto()
    GE = auto()
    NOT = auto()
    AND = auto()
    OR = auto()

    CHRs = auto()

    def __repr__(self):
        return f"<{self.name}>"


basicLineNr = 0

# | Operation   | Precedence |
# | ----------- | ---------- |
# | POWER       | 127        | x
# | MULT        | 124        | x
# | DIVIDE      | 124        | x
# | ADD         | 121        | x
# | SUB         | 121        | x
# | RELATIONALS | 100        |
# | NOT         | 90         |
# | AND         | 80         |
# | OR          | 70         |


def basicError(message):
    print(f"Error in BASIC line {basicLineNr}: {message}", file=sys.stderr)
    exit(1)


def parsePrimaryExpression(parts):
    if len(parts) == 0:
        basicError("Unexpected end of line")

    if isinstance(parts[0], Variable):
        expr = parts[0]
    elif isinstance(parts[0], float):
        expr = parts[0]
    elif isinstance(parts[0], StringLiteral):
        expr = parts[0]
    elif parts[0] == "(":
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
    elif parts[0] == Tokens["CHR$("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.CHRs, expr)
    else:
        basicError("Expected constant")

    return (expr, parts[1:])


def parsePowerExpression(parts):
    result, parts = parsePrimaryExpression(parts)

    while len(parts) > 0:
        if parts[0] == "^":
            rhs, parts = parsePowerExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = pow(result, rhs)
            else:
                result = (Operation.POW, result, rhs)
        else:
            break

    return (result, parts)


def parseUnaryExpression(parts):
    if len(parts) == 0:
        basicError("Unexpected end of line")

    if parts[0] == "-":
        expr, parts = parseUnaryExpression(parts[1:])
        if isinstance(expr, float):
            result = -expr
        else:
            result = (Operation.NEGATE, expr)

    elif parts[0] == "+":
        result, parts = parseUnaryExpression(parts[1:])

    else:
        result, parts = parsePowerExpression(parts)

    return (result, parts)


def parseMultiplicativeExpression(parts):
    result, parts = parseUnaryExpression(parts)

    while len(parts) > 0:
        if parts[0] == "*":
            rhs, parts = parsePrimaryExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result *= rhs
            else:
                result = (Operation.MULT, result, rhs)

        elif parts[0] == "/":
            rhs, parts = parsePrimaryExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result /= rhs
            else:
                result = (Operation.DIV, result, rhs)

        else:
            break

    return (result, parts)


def parseAdditiveExpression(parts):
    result, parts = parseMultiplicativeExpression(parts)

    while len(parts) > 0:
        if parts[0] == "+":
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = result + rhs
            else:
                result = (Operation.ADD, result, rhs)

        elif parts[0] == "-":
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = result - rhs
            else:
                result = (Operation.SUB, result, rhs)

        else:
            break

    return (result, parts)


def parseRelationalExpression(parts):
    result, parts = parseAdditiveExpression(parts)

    while len(parts) > 0:
        if parts[0] == "=":
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result == rhs else 0
            else:
                result = (Operation.EQ, result, rhs)

        elif parts[0] == Tokens["<>"]:
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result != rhs else 0
            else:
                result = (Operation.NE, result, rhs)

        elif parts[0] == "<":
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result < rhs else 0
            else:
                result = (Operation.LT, result, rhs)

        elif parts[0] == ">":
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result > rhs else 0
            else:
                result = (Operation.GT, result, rhs)

        elif parts[0] == Tokens["<="]:
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result <= rhs else 0
            else:
                result = (Operation.LE, result, rhs)

        elif parts[0] == Tokens[">="]:
            rhs, parts = parseRelationalExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1 if result >= rhs else 0
            else:
                result = (Operation.GE, result, rhs)

        else:
            break

    return (result, parts)


def parseNotExpression(parts):
    if len(parts) == 0:
        basicError("Unexpected end of line")

    if parts[0] == Tokens.NOT:
        expr, parts = parseNotExpression(parts[1:])
        if isinstance(expr, float):
            result = float(-int(expr) - 1)
        else:
            result = (Operation.NOT, expr)

    else:
        result, parts = parseRelationalExpression(parts)

    return (result, parts)


def parseAndExpression(parts):
    result, parts = parseNotExpression(parts)

    while len(parts) > 0:
        if parts[0] == Tokens.AND:
            rhs, parts = parseAndExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = float(int(result) & int(rhs))
            else:
                result = (Operation.AND, result, rhs)

        else:
            break

    return (result, parts)


def parseOrExpression(parts):
    result, parts = parseAndExpression(parts)

    while len(parts) > 0:
        if parts[0] == Tokens.OR:
            rhs, parts = parseOrExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = float(int(result) | int(rhs))
            else:
                result = (Operation.OR, result, rhs)

        else:
            break

    return (result, parts)


def parseExpression(parts):
    return parseOrExpression(parts)


def parseStatement(parts):
    if len(parts) > 0 and isinstance(parts[0], Variable):
        parts = [Statements.LET] + parts

    if len(parts) < 1 or not isinstance(parts[0], Statements):
        basicError("Expected statement")

    stmt = parts[0]

    if stmt == Statements.LET:
        if len(parts) < 4 or not isinstance(parts[1], Variable) or parts[2] != "=":
            basicError("Syntax error in variable assignment")

        name = parts[1]
        expr, parts = parseExpression(parts[3:])
        result = (Statements.LET, name, expr)

    elif stmt == Statements.END:
        result = Statements.END
        parts = parts[1:]

    elif stmt == Statements.GOTO:
        if len(parts) < 2 or not isinstance(parts[1], float):
            basicError("Syntax error in GOTO")
        result = (Statements.GOTO, parts[1])
        parts = parts[2:]

    elif stmt == Statements.PRINT:
        parts = parts[1:]

        exprs = []
        while len(parts) > 0:
            expr, parts = parseExpression(parts)
            exprs.append(expr)
        result = (Statements.PRINT, exprs)

    elif stmt == Statements.FOR:
        if len(parts) < 6 or not isinstance(parts[1], Variable) or parts[2] != "=":
            basicError("Syntax error")

        varName = parts[1]
        fromVal, parts = parseExpression(parts[3:])

        if len(parts) < 2 or parts[0] != Tokens.TO:
            basicError("Syntax error")

        toVal, parts = parseExpression(parts[1:])
        stepVal = 1.0

        if len(parts) > 1 and parts[0] == Tokens.STEP:
            stepVal, parts = parseExpression(parts[1:])

        result = (Statements.FOR, varName, fromVal, toVal, stepVal)

    elif stmt == Statements.IF:
        expr, parts = parseExpression(parts[1:])
        if len(parts) < 2 or parts[0] != Tokens.THEN:
            basicError("Syntax error")

        if isinstance(parts[1], float):
            st = (Statements.GOTO, parts[1])
            parts = parts[1:]
        else:
            print(parts)
            st, parts = parseStatement(parts[1:])

        result = (Statements.IF, expr, st)

    elif stmt == Statements.POKE:
        addr, parts = parseExpression(parts[1:])
        if len(parts) < 2 or parts[0] != ",":
            basicError("Syntax error")
        value, parts = parseExpression(parts[1:])

        result = (Statements.POKE, addr, value)

    elif stmt == Statements.NEXT:
        if len(parts) > 1 and isinstance(parts[1], Variable):
            result = (Statements.NEXT, parts[1])
            parts = parts[2:]
        else:
            result = (Statements.NEXT, None)
            parts = parts[1:]

    else:
        basicError(f"Syntax error: {parts}")

    return (result, parts)


programLines = tokenize(args.input.readlines())

for pl in programLines:
    basicLineNr = pl["lineNr"]
    print(f"- Line {basicLineNr}")

    for subLine in pl["subLines"]:
        result, parts = parseStatement(subLine)
        print(f"  - {result}")
