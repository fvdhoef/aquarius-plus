#!/usr/bin/env python3

with open("zout/bootrom.cim", "rb") as f:
    data = f.read()

with open("../../fpga/src/bootrom.v", "w") as f:
    print("module bootrom(", file=f)
    print("    input  wire [7:0] addr,", file=f)
    print("    output reg  [7:0] data);\n", file=f)
    print("    always @(addr)", file=f)
    print("        case (addr)", file=f)
    for idx, val in enumerate(data):
        print(f"            8'h{idx:02X}: data <= 8'h{val:02X};", file=f)
    print("            default: data <= 8'h00; // NOP", file=f)
    print("        endcase\n", file=f)
    print("endmodule", file=f)
