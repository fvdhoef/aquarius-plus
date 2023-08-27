from enum import Enum, auto

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
        "USR(",
        "FRE(",
        "LPOS(",
        "POS(",
        "INT(",  # FP-lib: done
        "SGN(",  # FP-lib: done
        "ABS(",  # FP-lib: done
        "SQR(",  # FP-lib: done
        "RND(",  # FP-lib: done
        "LOG(",  # FP-lib: done
        "EXP(",  # FP-lib: done
        "COS(",  # FP-lib: done
        "SIN(",  # FP-lib: done
        "TAN(",  # FP-lib: done
        "ATN(",  # FP-lib: done
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

    SIN = auto()
    COS = auto()
    TAN = auto()
    ATN = auto()
    LOG = auto()
    EXP = auto()
    RND = auto()
    SQR = auto()
    ABS = auto()
    SGN = auto()
    INT = auto()

    CHRs = auto()

    def __repr__(self):
        return f"<{self.name}>"


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
