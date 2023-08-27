import sys
import re
from .defs import Statements, Tokens, Variable, StringLiteral


def error(inputFile, idx, message):
    print(f"{inputFile}:{idx+1} {message}", file=sys.stderr)
    exit(1)


def tokenize(inputFile):
    with open(inputFile) as f:
        lines = f.readlines()

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
            error(inputFile, idx, "Syntax error")
        lineNr = int(m.group())
        line = line[m.end() :]

        if lineNr < lastLineNr or lineNr > 65000:
            error(inputFile, idx, "Invalid line number")

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

        programLines.append((lineNr, subLines))

    return programLines
