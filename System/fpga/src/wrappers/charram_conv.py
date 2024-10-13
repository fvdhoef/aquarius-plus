#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser(
    description="Generate charram from binary file (2048 bytes)"
)
parser.add_argument("input", help="Binary input file")

args = parser.parse_args()

with open(args.input, "rb") as f:
    data = f.read()

data = data.ljust(2048, b"\0")[0:2048]
data_strs = [f"{val:02X}" for val in data]

with open("charram.v", "w") as f:
    f.write(
        """module charram(
    input  wire        clk1,
    input  wire [10:0] addr1,
    output wire  [7:0] rddata1,
    input  wire  [7:0] wrdata1,
    input  wire        wren1,

    input  wire        clk2,
    input  wire [10:0] addr2,
    output wire  [7:0] rddata2);

    RAMB16_S9_S9 #(
        .INIT_A(9'h000),                // Value of output RAM registers on Port A at startup
        .INIT_B(9'h000),                // Value of output RAM registers on Port B at startup
        .SRVAL_A(9'h000),               // Port A output value upon SSR assertion
        .SRVAL_B(9'h000),               // Port B output value upon SSR assertion
        .WRITE_MODE_A("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .WRITE_MODE_B("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .SIM_COLLISION_CHECK("NONE"),   // "NONE", "WARNING_ONLY", "GENERATE_X_ONLY", "ALL"

        // The following INIT_xx declarations specify the initial contents of the RAM
"""
    )

    for i in range(0, 64):
        print(
            f"        .INIT_{i:02X}(256'h{''.join(list(reversed(data_strs[32*i:32*(i+1)])))}){',' if i<63 else ''}",
            file=f,
        )

    f.write(
        """    )
    
    RAMB16_S9_S9_inst(
        .CLKA(clk1),
        .SSRA(1'b0),
        .ADDRA(addr1),
        .DOA(rddata1),
        .DOPA(),
        .DIA(wrdata1),
        .DIPA(1'b0),
        .ENA(1'b1),
        .WEA(wren1),

        .CLKB(clk2),
        .SSRB(1'b0),
        .ADDRB(addr2),
        .DOB(rddata2),
        .DOPB(),
        .DIB(8'b0),
        .DIPB(1'b0),
        .ENB(1'b1),
        .WEB(1'b0)
    );

endmodule
"""
    )
