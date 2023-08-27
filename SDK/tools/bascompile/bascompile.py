#!/usr/bin/env python3

import argparse
import sys
from lib.defs import Statements, Tokens, Operation, Variable, StringLiteral
from lib.tokenizer import tokenize
from lib.parser import parse

parser = argparse.ArgumentParser(description="Convert BASIC text file to executable")
parser.add_argument("input", help="Input file")
# parser.add_argument("output", help="Output file", type=argparse.FileType("wb"))
args = parser.parse_args()

lines = parse(args.input)

for line in lines:
    print(line)
