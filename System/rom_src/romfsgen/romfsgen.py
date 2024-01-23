#!/usr/bin/env python3

import argparse
import pathlib
import struct

parser = argparse.ArgumentParser(prog="romfsgen", description="Generate romfs file")
parser.add_argument("output")
parser.add_argument("files", nargs="*")
args = parser.parse_args()

offset = 0
hdr_size = 1
files = []

try:
    for file in args.files:
        path = pathlib.Path(file)
        if not path.is_file():
            raise Exception(f"Expected file ({file})")
        st = path.stat()

        file = (path, path.name.encode() + b"\0", offset, st.st_size, int(st.st_mtime))
        files.append(file)

        hdr_size += 1 + 4 + 4 + 4 + len(file[1])

    with open(args.output, "wb") as f:
        for file in files:
            record = bytearray()
            record.extend(
                struct.pack(
                    "<BIII",
                    1 + 4 + 4 + 4 + len(file[1]),  # Size of record
                    file[2] + hdr_size,  # Offset to file contents
                    file[3],  # File size
                    file[4],  # Modified time
                )
            )
            record.extend(file[1])
            f.write(record)

        # Terminate with 0-sized record
        f.write(b"\0")

        # Output content of files
        for file in files:
            with open(file[0], "rb") as fi:
                f.write(fi.read())

except Exception as e:
    print(e)
