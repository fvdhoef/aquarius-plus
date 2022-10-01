#!/usr/bin/env python3

#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser(description="Generate verilog bootrom from binary")
parser.add_argument("input", help="Binary input file")
parser.add_argument("output", help="Verilog output file")

args = parser.parse_args()

with open(args.input, "rb") as f:
    data = f.read()


with open(args.input, "rb") as f:
    data = f.read()

with open(args.output, "w") as f:
    print("module bootrom(", file=f)
    print("    input  wire        clk,", file=f)
    print("    input  wire [12:0] addr,", file=f)
    print("    output reg   [7:0] rddata);\n", file=f)
    print("    always @(posedge(clk))", file=f)
    print("        case (addr)", file=f)
    for idx, val in enumerate(data):
        print(f"            13'h{idx:04X}: rddata <= 8'h{val:02X};", file=f)
    print("            default:  rddata <= 8'h00; // NOP", file=f)
    print("        endcase\n", file=f)
    print("endmodule", file=f)
