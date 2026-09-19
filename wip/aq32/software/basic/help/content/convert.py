#!/usr/bin/env python3
import struct


files = {}
index_data = b""
content_data = b""


def read_topics():
    topics = []

    with open("_topics.txt", "rt") as f:
        for line in f.readlines():
            line = line.strip()
            if len(line) == 0:
                continue
            if line.startswith("#"):
                continue
            name, path = line.split(" ", 1)
            path = path.strip()
            topics.append((name, path))

    topics.sort()
    return topics


def process_file(path):
    global files
    global content_data

    if path in files:
        return

    data = bytearray()
    num_lines = 0

    with open(path, "rt") as f:
        for idx, line in enumerate(f.readlines()):
            line = line.rstrip()
            # if len(line) > 78:
            #     print(f"{path}:{idx+1} Line too long {len(line)} > 78!")
            #     exit(1)

            try:
                enc = line.encode("ascii")

                line_data = bytearray()
                idx = 0
                while idx < len(enc):
                    val = enc[idx]
                    if val == b"\\"[0] and enc[idx + 1] == b"x"[0]:
                        # Hexadecimal value
                        val = int(enc[idx + 2 : idx + 4], 16)
                        idx += 3

                    line_data.append(val)
                    idx = idx + 1

                data.append(len(line_data))
                data.extend(line_data)

            except:
                print(f"{path}:{idx+1} Encoding error")
                exit(1)
            num_lines += 1

    data = struct.pack("<H", num_lines) + data
    data = struct.pack("<H", len(data)) + data

    offset = len(content_data)
    content_data += data

    files[path] = offset


topics = read_topics()
index_data += struct.pack("<H", len(topics))

for topic in topics:
    process_file(topic[1])

    offset = files[topic[1]]
    index_data += (
        struct.pack("<B", len(topic[0]))
        + topic[0].encode("latin-1")
        + struct.pack("<I", offset)
    )

with open("basic.hlp", "wb") as f:
    f.write(b"HELP" + struct.pack("<H", len(index_data)) + index_data + content_data)
