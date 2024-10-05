#!/bin/env python3
import os
import struct
import argparse
import subprocess


parser = argparse.ArgumentParser()
parser.add_argument("in_file")
parser.add_argument("out_file")
args = parser.parse_args()


st = os.stat(args.in_file)

result = subprocess.run(
    ["xz", "-v", "--check=crc32", "--lzma2=dict=8KiB", "-e", "-c", args.in_file],
    capture_output=True,
)
print(result.stderr.decode(), end="")

with open(args.out_file, "wb") as f:
    f.write(struct.pack("<I", st.st_size))
    f.write(result.stdout)
