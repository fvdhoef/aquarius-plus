#!/usr/bin/env python3
import argparse
import sys

ROMSIZE = 16384

parser = argparse.ArgumentParser(description="Convert binary file into verilog ROM")
parser.add_argument("input", help="Binary input file", type=argparse.FileType("rb"))
parser.add_argument("output", help="Verilog output file", type=argparse.FileType("w"))
args = parser.parse_args()

data = args.input.read()
if len(data) != ROMSIZE:
    print(
        f"Input should be {ROMSIZE} bytes long, but is {len(data)} bytes",
        file=sys.stderr,
    )
    exit(1)

print(
    """module rom(
    input  wire        clk,
    input  wire [13:0] addr,
    output reg   [7:0] rddata);

    always @(posedge clk)
        case (addr)""",
    file=args.output,
)

for i in range(0, ROMSIZE, 4):
    print(
        f"            14'h{i+0:04X}: rddata <= 8'h{data[i+0]:02X}; 14'h{i+1:04X}: rddata <= 8'h{data[i+1]:02X}; 14'h{i+2:04X}: rddata <= 8'h{data[i+2]:02X}; 14'h{i+3:04X}: rddata <= 8'h{data[i+3]:02X};",
        file=args.output,
    )

print(
    """        endcase

endmodule""",
    file=args.output,
)
