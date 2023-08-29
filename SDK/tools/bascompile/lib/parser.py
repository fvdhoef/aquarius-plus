import sys
from .defs import Statements, Tokens, Operation, Variable, StringLiteral
from .tokenizer import tokenize


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
    elif parts[0] == Tokens["SIN("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.SIN, expr)
    elif parts[0] == Tokens["COS("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.COS, expr)
    elif parts[0] == Tokens["TAN("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.TAN, expr)
    elif parts[0] == Tokens["ATN("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.ATN, expr)
    elif parts[0] == Tokens["LOG("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.LOG, expr)
    elif parts[0] == Tokens["EXP("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.EXP, expr)
    elif parts[0] == Tokens["RND("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.RND, expr)
    elif parts[0] == Tokens["SQR("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.SQR, expr)
    elif parts[0] == Tokens["ABS("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.ABS, expr)
    elif parts[0] == Tokens["SGN("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.SGN, expr)
    elif parts[0] == Tokens["INT("]:
        expr, parts = parseExpression(parts[1:])
        if len(parts) == 0 or parts[0] != ")":
            basicError("Expected ')'")
        expr = (Operation.INT, expr)

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
            rhs, parts = parsePrimaryExpression(parts[1:])
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
            rhs, parts = parseUnaryExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result *= rhs
            else:
                result = (Operation.MULT, result, rhs)

        elif parts[0] == "/":
            rhs, parts = parseUnaryExpression(parts[1:])
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
            rhs, parts = parseMultiplicativeExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = result + rhs
            else:
                result = (Operation.ADD, result, rhs)

        elif parts[0] == "-":
            rhs, parts = parseMultiplicativeExpression(parts[1:])
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
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result == rhs else 0.0
            else:
                result = (Operation.EQ, result, rhs)

        elif parts[0] == Tokens["<>"]:
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result != rhs else 0.0
            else:
                result = (Operation.NE, result, rhs)

        elif parts[0] == "<":
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result < rhs else 0.0
            else:
                result = (Operation.LT, result, rhs)

        elif parts[0] == ">":
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result > rhs else 0.0
            else:
                result = (Operation.GT, result, rhs)

        elif parts[0] == Tokens["<="]:
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result <= rhs else 0.0
            else:
                result = (Operation.LE, result, rhs)

        elif parts[0] == Tokens[">="]:
            rhs, parts = parseAdditiveExpression(parts[1:])
            if isinstance(result, float) and isinstance(rhs, float):
                result = -1.0 if result >= rhs else 0.0
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
            rhs, parts = parseNotExpression(parts[1:])
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
            rhs, parts = parseAndExpression(parts[1:])
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
        result = (Statements.END,)
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
            if parts[0] == ";":
                parts = parts[1:]
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
            parts = parts[2:]
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


def parse(inputFile):
    (programLines, variables) = tokenize(inputFile)

    result = []

    for pl in programLines:
        global basicLineNr
        basicLineNr = pl[0]

        stmts = []
        for subLine in pl[1]:
            stmt, parts = parseStatement(subLine)
            if len(parts) != 0:
                print(parts)
                basicError("Unexpected characters at end of line")

            # print(f"  - {result}")

            stmts.append(stmt)

        result.append((basicLineNr, stmts))

    return (result, variables)
