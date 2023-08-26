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


class Operation(Enum):
    NEGATE = auto()
    MULT = auto()
    DIV = auto()
    ADD = auto()
    SUB = auto()

    def __repr__(self):
        return f"<{self.name}>"


class Variable:
    def __init__(self, name):
        self.name = name

    def __repr__(self):
        return f"<Variable:{self.name}>"


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


basicLineNr = 0


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
        expr, parts = getExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
    else:
        basicError("Expected constant")

    return (expr, parts[1:])


def parseUnaryExpression(parts):
    if len(parts) == 0:
        basicError("Unexpected end of line")

    if parts[0] == "-":
        expr, parts = parseUnaryExpression(parts[1:])
        if isinstance(expr, float):
            result = -expr
        else:
            result = ((Operation.NEGATE, expr), parts)

    elif parts[0] == "+":
        result, parts = parseUnaryExpression(parts[1:])

    else:
        result, parts = parsePrimaryExpression(parts)

    return (result, parts)


def parseMultiplicativeExpression(parts):
    result, parts = parseUnaryExpression(parts)
    print("!!!!?", result)

    while len(parts) > 0:
        if parts[0] == "*":
            rhs, parts = parsePrimaryExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result *= rhs
            else:
                print("!!!!!", result)

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


def getExpression(parts):
    return parseAdditiveExpression(parts)


programLines = tokenize(args.input.readlines())

for pl in programLines:
    basicLineNr = pl["lineNr"]
    print(basicLineNr)

    for subLine in pl["subLines"]:
        if isinstance(subLine[0], Variable):
            subLine = [Statements.LET] + subLine

        if subLine[0] == Statements.LET:
            if (
                len(subLine) < 4
                or not isinstance(subLine[1], Variable)
                or subLine[2] != "="
            ):
                error(pl["lineNr"], "Syntax error in variable assignment")

            name = subLine[1].name

            expr, subLine = getExpression(subLine[3:])
            if len(subLine) > 0:
                basicError("Unexpected characters")

            print(f"--> Let {name} = {expr}")

        if subLine:
            print("-", subLine)

        # print(type(subLine[0]))
