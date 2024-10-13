#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser(description="Convert binary file into verilog ROM")
parser.add_argument("input", help="Binary input file", type=argparse.FileType("rb"))
parser.add_argument("output", help="Verilog output file", type=argparse.FileType("w"))
args = parser.parse_args()
f = args.output

# Read input file
data = args.input.read()

print("""`default_nettype none
`timescale 1 ns / 1 ps

module rom(
    input  wire        clk,
    input  wire [12:0] addr,
    output reg   [7:0] rddata
);

    always @(posedge clk) case (addr)""", file=f)

for i in range(len(data)):
    print(f"        13'h{i:02X}: rddata <= 8'h{data[i]:02X};", file=f)
print("        default:  rddata <= 8'h00;", file=f)

print("    endcase", file=f)
print("\nendmodule", file=f)
