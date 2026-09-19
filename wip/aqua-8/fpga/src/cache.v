`default_nettype none
`timescale 1 ns / 1 ps

module cache(
    input  wire        clk,
    input  wire        reset,

    // Slave bus interface (from CPU)
    input  wire [16:0] s_i_addr,
    input  wire        s_i_flush,
    input  wire        s_i_strobe,
    output wire        s_i_wait,
    output wire [31:0] s_i_rddata,

    input  wire [16:0] s_d_addr,
    input  wire [31:0] s_d_wrdata,
    input  wire  [3:0] s_d_bytesel,
    input  wire        s_d_wren,
    input  wire        s_d_flush,
    input  wire        s_d_strobe,
    output wire        s_d_wait,
    output wire [31:0] s_d_rddata,

    // Memory command interface
    output wire [16:0] m_addr,
    output wire [31:0] m_wrdata,
    output wire        m_wren,
    output wire        m_strobe,
    input  wire        m_wait,
    input  wire [31:0] m_rddata
);

    wire [16:0] m_i_addr;
    wire        m_i_strobe;
    wire        m_i_wait;

    wire [16:0] m_d_addr;
    wire [31:0] m_d_wrdata;
    wire        m_d_wren;
    wire        m_d_strobe;
    wire        m_d_wait;

    icache icache(
        .clk(clk),
        .reset(reset),

        // Slave bus interface (from CPU)
        .s_addr(s_i_addr),
        .s_flush(s_i_flush),
        .s_strobe(s_i_strobe),
        .s_wait(s_i_wait),
        .s_rddata(s_i_rddata),

        // Memory command interface
        .m_addr(m_i_addr),
        .m_strobe(m_i_strobe),
        .m_wait(m_i_wait),
        .m_rddata(m_rddata));

    dcache dcache(
        .clk(clk),
        .reset(reset),

        // Slave bus interface (from CPU)
        .s_addr(s_d_addr),
        .s_wrdata(s_d_wrdata),
        .s_bytesel(s_d_bytesel),
        .s_wren(s_d_wren),
        .s_flush(s_d_flush),
        .s_strobe(s_d_strobe),
        .s_wait(s_d_wait),
        .s_rddata(s_d_rddata),

        // Memory command interface
        .m_addr(m_d_addr),
        .m_wrdata(m_d_wrdata),
        .m_wren(m_d_wren),
        .m_strobe(m_d_strobe),
        .m_wait(m_d_wait),
        .m_rddata(m_rddata));

    reg d_cur_master, q_cur_master;     // 0:I, 1:D

    wire allow_change = 
        (!q_cur_master && !m_i_strobe) ||
        ( q_cur_master && !m_d_strobe);

    always @* begin
        d_cur_master = q_cur_master;
        if (allow_change) begin
            if (m_d_strobe)
                d_cur_master = 1;
            else if (m_i_strobe)
                d_cur_master = 0;
        end
    end

    assign m_wrdata = m_d_wrdata;

    assign m_i_wait = !d_cur_master ? m_wait : 1;
    assign m_d_wait =  d_cur_master ? m_wait : 1;
    assign m_addr   = !d_cur_master ? m_i_addr : m_d_addr;
    assign m_wren   =  d_cur_master ? m_d_wren : 0;
    assign m_strobe = !d_cur_master ? m_i_strobe : m_d_strobe;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_cur_master <= 0;
        end else begin
            q_cur_master <= d_cur_master;
        end

endmodule
