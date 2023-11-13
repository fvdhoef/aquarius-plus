module linebuf(
    input  wire        clk,

    input  wire        linesel,

    input  wire  [7:0] idx1,
    output wire  [7:0] rddata1,
    input  wire  [7:0] wrdata1,
    input  wire        wren1,

    input  wire  [7:0] idx2,
    output wire  [4:0] rddata2);

    wire [10:0] addr1 = {2'b0,  linesel, idx1};
    wire [10:0] addr2 = {2'b0, !linesel, idx2};

    wire [7:0] lb_rddata2;
    assign rddata2 = lb_rddata2[4:0];

    RAMB16_S9_S9 #(
        .INIT_A(9'h000),                // Value of output RAM registers on Port A at startup
        .INIT_B(9'h000),                // Value of output RAM registers on Port B at startup
        .SRVAL_A(9'h000),               // Port A output value upon SSR assertion
        .SRVAL_B(9'h000),               // Port B output value upon SSR assertion
        .WRITE_MODE_A("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .WRITE_MODE_B("WRITE_FIRST"),   // WRITE_FIRST, READ_FIRST or NO_CHANGE
        .SIM_COLLISION_CHECK("NONE")    // "NONE", "WARNING_ONLY", "GENERATE_X_ONLY", "ALL"
    )
   
    RAMB16_S9_S9_inst(
        .CLKA(clk),
        .SSRA(1'b0),
        .ADDRA(addr1),
        .DOA(rddata1),
        .DOPA(),
        .DIA(wrdata1),
        .DIPA(1'b0),
        .ENA(1'b1),
        .WEA(wren1),

        .CLKB(clk),
        .SSRB(1'b0),
        .ADDRB(addr2),
        .DOB(lb_rddata2),
        .DOPB(),
        .DIB(8'b0),
        .DIPB(1'b0),
        .ENB(1'b1),
        .WEB(1'b0)
    );

endmodule
