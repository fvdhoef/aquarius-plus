#!/usr/bin/env python3
import argparse
import struct

parser = argparse.ArgumentParser(description="Convert binary file into verilog ROM")
parser.add_argument("input", help="Binary input file", type=argparse.FileType("rb"))
parser.add_argument("output", help="Verilog output file", type=argparse.FileType("w"))
args = parser.parse_args()
f = args.output

# Read input file
data = args.input.read()


print(
    """`default_nettype none
`timescale 1 ns / 1 ps

module bootrom(
    input  wire        clk,
    input  wire  [8:0] addr,
    output reg  [31:0] rddata
);

    always @(posedge clk) case (addr)""",
    file=f,
)

for i in range(0, len(data), 4):
    val = struct.unpack_from("<I", data, i)[0]
    print(
        f"        9'h{i:03X}:  rddata <= 32'h{val:08X};",
        file=f,
    )
print("        default: rddata <= 32'h00000000;", file=f)

print("    endcase", file=f)
print("\nendmodule", file=f)
