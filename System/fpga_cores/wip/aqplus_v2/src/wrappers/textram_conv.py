#!/usr/bin/env python3
import argparse

parser = argparse.ArgumentParser(
    description="Generate textram from binary .SCR file (2048 bytes)"
)
parser.add_argument("input", help="Binary input file")

args = parser.parse_args()

with open(args.input, "rb") as f:
    data = f.read()

data = data.ljust(2048, b"\0")[0:2048]

# Reorder data
data2 = [data[i] | (data[i + 1024] << 8) for i in range(1024)]
# data[0:1024]. data[1024:2048]

bla = [f"{val:04X}" for val in data2]

chunk = bla[0:16]

with open("textram.v", "w") as f:
    f.write(
        """module textram(
    // First port - CPU access
    input  wire        p1_clk,
    input  wire [10:0] p1_addr,
    output wire  [7:0] p1_rddata,
    input  wire  [7:0] p1_wrdata,
    input  wire        p1_wren,

    // Second port - Video access
    input  wire        p2_clk,
    input  wire  [9:0] p2_addr,
    output wire [15:0] p2_rddata);

    RAMB16_S9_S18 #(
        .INIT_A(9'h000),                // Value of output RAM registers on Port A at startup
        .INIT_B(18'h00000),             // Value of output RAM registers on Port B at startup
        .SRVAL_A(9'h000),               // Port A output value upon SSR assertion
        .SRVAL_B(18'h00000),            // Port B output value upon SSR assertion
        .WRITE_MODE_A("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .WRITE_MODE_B("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .SIM_COLLISION_CHECK("NONE"),   // "NONE", "WARNING_ONLY", "GENERATE_X_ONLY", "ALL"

        // The following INIT_xx declarations specify the initial contents of the RAM
"""
    )

    def gen(start, stop):
        print(
            f"        // Port A Address {start * 32} to {stop * 32 - 1}, Port B Address {start * 16} to {stop * 16 - 1}",
            file=f,
        )
        for i in range(start, stop):
            print(
                f"        .INIT_{i:02X}(256'h{''.join(list(reversed(bla[16*i:16*(i+1)])))}),",
                file=f,
            )
        print("", file=f)

    for i in range(4):
        gen(i * 16, (i + 1) * 16)

    f.write(
        """        // The next set of INITP_xx are for the parity bits
        // Port A Address 0 to 511, Port B Address 0 to 255
        .INITP_00(256'h0000000000000000000000000000000000000000000000000000000000000000),
        .INITP_01(256'h0000000000000000000000000000000000000000000000000000000000000000),
        // Port A Address 512 to 1023, Port B Address 256 to 511
        .INITP_02(256'h0000000000000000000000000000000000000000000000000000000000000000),
        .INITP_03(256'h0000000000000000000000000000000000000000000000000000000000000000),
        // Port A Address 1024 to 1535, Port B Address 512 to 767
        .INITP_04(256'h0000000000000000000000000000000000000000000000000000000000000000),
        .INITP_05(256'h0000000000000000000000000000000000000000000000000000000000000000),
        // Port A Address 1536 to 2047, Port B Address 768 to 1024
        .INITP_06(256'h0000000000000000000000000000000000000000000000000000000000000000),
        .INITP_07(256'h0000000000000000000000000000000000000000000000000000000000000000))
    
    RAMB16_S9_S18_inst(
        // Port-A
        .CLKA(p1_clk),      // Clock
        .SSRA(1'b0),        // Synchronous Set/Reset Input
        .ADDRA(p1_addr),    // 11-bit Address Input
        .DOA(p1_rddata),    // 8-bit Data Output
        .DOPA(),            // 1-bit Parity Output
        .DIA(p1_wrdata),    // 8-bit Data Input
        .DIPA(1'b0),        // 1-bit parity Input
        .ENA(1'b1),         // RAM Enable Input
        .WEA(p1_wren),      // Write Enable Input

        // Port-B
        .CLKB(p2_clk),      // Clock
        .SSRB(1'b0),        // Synchronous Set/Reset Input
        .ADDRB(p2_addr),    // 10-bit Address Input
        .DOB(p2_rddata),    // 16-bit Data Output
        .DOPB(),            // 2-bit Parity Output
        .DIB(16'b0),        // 16-bit Data Input
        .DIPB(2'b0),        // 2-bit parity Input
        .ENB(1'b1),         // RAM Enable Input
        .WEB(1'b0)          // Write Enable Input
    );

endmodule
"""
    )
