`default_nettype none
`timescale 1 ns / 1 ps

module vram(
    // First port - CPU access
    input  wire        p1_clk,
    input  wire [13:0] p1_addr,
    output wire  [7:0] p1_rddata,
    input  wire  [7:0] p1_wrdata,
    input  wire        p1_wren,

    // Second port - Video access
    input  wire        p2_clk,
    input  wire [12:0] p2_addr,
    output wire [15:0] p2_rddata);

    assign p1_rddata = 8'b0;
    assign p2_rddata = 16'b0;

    // generate
    //     genvar i;
    //     for (i=0; i<8; i=i+1) begin: vram_gen
    //         RAMB16_S1_S2 #(.SIM_COLLISION_CHECK("NONE")) RAMB16_S1_S2_inst(
    //             // Port-A
    //             .CLKA(p1_clk),          // Clock
    //             .SSRA(1'b0),            // Synchronous Set/Reset Input
    //             .ADDRA(p1_addr),        // 14-bit Address Input
    //             .DOA(p1_rddata[i]),     // 1-bit Data Output
    //             .DIA(p1_wrdata[i]),     // 1-bit Data Input
    //             .ENA(1'b1),             // RAM Enable Input
    //             .WEA(p1_wren),          // Write Enable Input

    //             // Port-B
    //             .CLKB(p2_clk),              // Clock
    //             .SSRB(1'b0),                // Synchronous Set/Reset Input
    //             .ADDRB(p2_addr),            // 13-bit Address Input
    //             .DOB({p2_rddata[i+8], p2_rddata[i]}), // 2-bit Data Output
    //             .DIB(2'b0),                 // 2-bit Data Input
    //             .ENB(1'b1),                 // RAM Enable Input
    //             .WEB(1'b0)                  // Write Enable Input
    //         );
    //     end
    // endgenerate

endmodule
