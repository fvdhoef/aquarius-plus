#!/usr/bin/env python3
import argparse
import struct
import math

fm_logsin_rom = [
    round(-256 * math.log2(math.sin((x + 0.5) / 256 * (math.pi / 2))))
    for x in range(256)
]

fm_exp_rom = [round((math.pow(2, (255 - x) / 256) - 1) * 1024) for x in range(256)]

fm_pan_l_rom = [int(255 * math.cos(math.pi / 2 * max(0, x - 1) / 126)) for x in range(128)]
fm_pan_r_rom = [int(255 * math.sin(math.pi / 2 * max(0, x - 1) / 126)) for x in range(128)]
fm_pan_l_rom[0] = 0
fm_pan_r_rom[0] = 0

f = open("fm_logsin_rom.v", "wt")

print(
    """`default_nettype none
`timescale 1 ns / 1 ps

(* rom_style = "distributed" *)
module fm_logsin_rom(
    input  wire  [7:0] idx,
    output reg  [11:0] value
);

    always @* case (idx)""",
    file=f,
)

for i in range(256):
    print(f"        8'h{i:02X}: value = 12'h{fm_logsin_rom[i]:03x};", file=f)

print("        default: value = 12'h0;", file=f)
print("    endcase", file=f)
print("\nendmodule", file=f)
f.close()


f = open("fm_exp_rom.v", "wt")

print(
    """`default_nettype none
`timescale 1 ns / 1 ps

(* rom_style = "distributed" *)
module fm_exp_rom(
    input  wire  [7:0] idx,
    output reg   [9:0] value
);

    always @* case (idx)""",
    file=f,
)

for i in range(256):
    print(f"        8'h{i:02X}: value = 10'h{fm_exp_rom[i]:03x};", file=f)

print("        default: value = 10'h0;", file=f)
print("    endcase", file=f)
print("\nendmodule", file=f)
f.close()


f = open("fm_pan_l_rom.v", "wt")

print(
    """`default_nettype none
`timescale 1 ns / 1 ps

(* rom_style = "distributed" *)
module fm_pan_l_rom(
    input  wire  [6:0] idx,
    output reg   [7:0] value
);

    always @* case (idx)""",
    file=f,
)

for i in range(128):
    print(f"        7'h{i:02X}: value = 8'h{fm_pan_l_rom[i]:02x};", file=f)

print("        default: value = 8'h0;", file=f)
print("    endcase", file=f)
print("\nendmodule", file=f)
f.close()


f = open("fm_pan_r_rom.v", "wt")

print(
    """`default_nettype none
`timescale 1 ns / 1 ps

(* rom_style = "distributed" *)
module fm_pan_r_rom(
    input  wire  [6:0] idx,
    output reg   [7:0] value
);

    always @* case (idx)""",
    file=f,
)

for i in range(128):
    print(f"        7'h{i:02X}: value = 8'h{fm_pan_r_rom[i]:02x};", file=f)

print("        default: value = 8'h0;", file=f)
print("    endcase", file=f)
print("\nendmodule", file=f)
f.close()
