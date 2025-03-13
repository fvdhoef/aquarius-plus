`default_nettype none
`timescale 1 ns / 1 ps

module textram(
    // First port - CPU access
    input  wire        p1_clk,
    input  wire [10:0] p1_addr,
    output wire [15:0] p1_rddata,
    input  wire [15:0] p1_wrdata,
    input  wire        p1_wren,

    // Second port - Video access
    input  wire        p2_clk,
    input  wire [10:0] p2_addr,
    output wire [15:0] p2_rddata);

    wire [0:0] p1_rddata_p_ram0, p1_rddata_p_ram1;
    wire [0:0] p2_rddata_p_ram0, p2_rddata_p_ram1;

    RAMB16_S9_S9 #(
        .INIT_A(9'h000),                // Value of output RAM registers on Port A at startup
        .INIT_B(9'h000),                // Value of output RAM registers on Port B at startup
        .SRVAL_A(9'h000),               // Port A output value upon SSR assertion
        .SRVAL_B(9'h000),               // Port B output value upon SSR assertion
        .WRITE_MODE_A("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .WRITE_MODE_B("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .SIM_COLLISION_CHECK("NONE")    // "NONE", "WARNING_ONLY", "GENERATE_X_ONLY", "ALL"
    )
    
    ram0(
        // Port-A
        .CLKA(p1_clk),                  // Clock
        .SSRA(1'b0),                    // Synchronous Set/Reset Input
        .ADDRA(p1_addr),                // 11-bit Address Input
        .DOA(p1_rddata[7:0]),           // 8-bit Data Output
        .DOPA(p1_rddata_p_ram0),        // 1-bit Parity Output
        .DIA(p1_wrdata[7:0]),           // 8-bit Data Input
        .DIPA(1'b0),                    // 1-bit parity Input
        .ENA(1'b1),                     // RAM Enable Input
        .WEA(p1_wren),                  // Write Enable Input

        // Port-B
        .CLKB(p2_clk),                  // Clock
        .SSRB(1'b0),                    // Synchronous Set/Reset Input
        .ADDRB(p2_addr),                // 10-bit Address Input
        .DOB(p2_rddata[7:0]),           // 8-bit Data Output
        .DOPB(p2_rddata_p_ram0),        // 1-bit Parity Output
        .DIB(8'b0),                     // 8-bit Data Input
        .DIPB(1'b0),                    // 1-bit parity Input
        .ENB(1'b1),                     // RAM Enable Input
        .WEB(1'b0)                      // Write Enable Input
    );

    RAMB16_S9_S9 #(
        .INIT_A(9'h000),                // Value of output RAM registers on Port A at startup
        .INIT_B(9'h000),                // Value of output RAM registers on Port B at startup
        .SRVAL_A(9'h000),               // Port A output value upon SSR assertion
        .SRVAL_B(9'h000),               // Port B output value upon SSR assertion
        .WRITE_MODE_A("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .WRITE_MODE_B("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .SIM_COLLISION_CHECK("NONE")    // "NONE", "WARNING_ONLY", "GENERATE_X_ONLY", "ALL"
    )
    
    ram1(
        // Port-A
        .CLKA(p1_clk),                  // Clock
        .SSRA(1'b0),                    // Synchronous Set/Reset Input
        .ADDRA(p1_addr),                // 11-bit Address Input
        .DOA(p1_rddata[15:8]),          // 8-bit Data Output
        .DOPA(p1_rddata_p_ram1),        // 1-bit Parity Output
        .DIA(p1_wrdata[15:8]),          // 8-bit Data Input
        .DIPA(1'b0),                    // 1-bit parity Input
        .ENA(1'b1),                     // RAM Enable Input
        .WEA(p1_wren),                  // Write Enable Input

        // Port-B
        .CLKB(p2_clk),                  // Clock
        .SSRB(1'b0),                    // Synchronous Set/Reset Input
        .ADDRB(p2_addr),                // 10-bit Address Input
        .DOB(p2_rddata[15:8]),          // 8-bit Data Output
        .DOPB(p2_rddata_p_ram1),        // 1-bit Parity Output
        .DIB(8'b0),                     // 8-bit Data Input
        .DIPB(1'b0),                    // 1-bit parity Input
        .ENB(1'b1),                     // RAM Enable Input
        .WEB(1'b0)                      // Write Enable Input
    );

endmodule
