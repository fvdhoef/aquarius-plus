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
        "<>",  # Done
        "<=",  # Done
        ">=",  # Done
        "TAB(",
        "TO",
        "SPC(",
        "INKEY$",
        "THEN",
        "STEP",
        "AND",
        "OR",
        "USR(",
        "FRE(",
        "LPOS(",
        "POS(",
        "NOT",  # Done
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
    NEGATE = auto()  # Done
    MULT = auto()  # Done
    DIV = auto()  # Done
    ADD = auto()  # Done
    SUB = auto()  # Done
    POW = auto()  # Done
    EQ = auto()  # Done
    NE = auto()  # Done
    LT = auto()  # Done
    LE = auto()  # Done
    GT = auto()  # Done
    GE = auto()  # Done
    NOT = auto()  # Done
    AND = auto()  # Done
    OR = auto()  # Done

    SIN = auto()  # Done
    COS = auto()  # Done
    TAN = auto()  # Done
    ATN = auto()  # Done
    LOG = auto()  # Done
    EXP = auto()  # Done
    RND = auto()  # Done
    SQR = auto()  # Done
    ABS = auto()  # Done
    SGN = auto()  # Done
    INT = auto()  # Done

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
