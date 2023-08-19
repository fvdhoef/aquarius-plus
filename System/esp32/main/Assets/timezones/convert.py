#!/usr/bin/env python3

import struct
import os
from pathlib import Path


def parseFile(path):
    f = open(path, "rb")

    if f.read(4) != b"TZif":
        raise RuntimeError("Incorrect file")
    version = f.read(1)
    if version != b"2" and version != b"3":
        raise RuntimeError("Incorrect version")

    f.seek(15, 1)

    (
        tzh_ttisgmtcnt,
        tzh_ttisstdcnt,
        tzh_leapcnt,
        tzh_timecnt,
        tzh_typecnt,
        tzh_charcnt,
    ) = struct.unpack(">IIIIII", f.read(4 * 6))

    # print("tzh_ttisgmtcnt", tzh_ttisgmtcnt)
    # print("tzh_ttisstdcnt", tzh_ttisstdcnt)
    # print("tzh_leapcnt", tzh_leapcnt)
    # print("tzh_timecnt", tzh_timecnt)
    # print("tzh_typecnt", tzh_typecnt)
    # print("tzh_charcnt", tzh_charcnt)

    f.seek(tzh_timecnt * 4, 1)
    f.seek(tzh_timecnt * 1, 1)
    f.seek(tzh_typecnt * 6, 1)
    f.seek(tzh_leapcnt * 4, 1)
    f.seek(tzh_ttisstdcnt * 1, 1)
    f.seek(tzh_ttisgmtcnt * 1, 1)
    f.seek(tzh_charcnt * 1, 1)

    if f.read(4) != b"TZif":
        raise RuntimeError("Incorrect file")
    version = f.read(1)
    if version != b"2" and version != b"3":
        raise RuntimeError("Incorrect version")
    f.seek(15, 1)

    (
        tzh_ttisgmtcnt,
        tzh_ttisstdcnt,
        tzh_leapcnt,
        tzh_timecnt,
        tzh_typecnt,
        tzh_charcnt,
    ) = struct.unpack(">IIIIII", f.read(4 * 6))

    # print("tzh_ttisgmtcnt", tzh_ttisgmtcnt)
    # print("tzh_ttisstdcnt", tzh_ttisstdcnt)
    # print("tzh_leapcnt", tzh_leapcnt)
    # print("tzh_timecnt", tzh_timecnt)
    # print("tzh_typecnt", tzh_typecnt)
    # print("tzh_charcnt", tzh_charcnt)

    f.seek(tzh_timecnt * 8, 1)
    f.seek(tzh_timecnt * 1, 1)
    f.seek(tzh_typecnt * 6, 1)
    f.seek(tzh_leapcnt * 4, 1)
    f.seek(tzh_ttisstdcnt * 1, 1)
    f.seek(tzh_ttisgmtcnt * 1, 1)
    f.seek(tzh_charcnt * 1, 1)

    if f.read(1) != b"\n":
        raise RuntimeError("Expected newline")

    return f.readline().strip().decode()


# print(parseFile("/etc/localtime"))
# print(os.walk("/usr/share/zoneinfo"))

for i in Path("/usr/share/zoneinfo").glob("*/*"):
    if not i.is_file() or "posix" in i.parts or "Etc" in i.parts or "right" in i.parts:
        continue

    name = "/".join(i.parts[4:])
    tzstr = parseFile(i)

    if tzstr == "":
        continue

    # print(f'{{"{name}", "{tzstr}"}},')

    print(f'{{"{tzstr}", "{name}"}},')
