`default_nettype none
`timescale 1 ns / 1 ps

module sram_cache(
    input  wire        clk,
    input  wire        reset,

    // Slave bus interface (from CPU)
    input  wire [16:0] s_addr,
    input  wire [31:0] s_wrdata,
    input  wire  [3:0] s_bytesel,
    input  wire        s_wren,
    input  wire        s_strobe,
    output wire        s_wait,
    output wire [31:0] s_rddata,

    // Memory command interface
    output wire [16:0] m_addr,
    output wire [31:0] m_wrdata,
    output wire        m_wren,
    output wire        m_strobe,
    input  wire        m_wait,
    input  wire [31:0] m_rddata
);

    // SRAM is 512KB
    //
    // Cache line size is 1 words (4 bytes), so SRAM is divided in 131072 lines.
    // Cache line is selected by s_addr[20:3].
    // Cache size 8KB (2048 cache lines)
    //
    // s_addr:
    // [10: 0] -> (11 bits) word within line
    // [16:11] ->  (6 bits) tag

    //////////////////////////////////////////////////////////////////////////
    // Cache data RAM
    //////////////////////////////////////////////////////////////////////////
    reg s_data_write;
    reg m_data_write;

    dpram8k cache_ram(
        .a_clk(clk), .a_addr(s_addr[10:0]), .a_wrdata(s_wrdata), .a_wrsel(s_bytesel), .a_wren(s_data_write), .a_rddata(s_rddata),
        .b_clk(clk), .b_addr(m_addr[10:0]), .b_wrdata(m_rddata), .b_wrsel(4'b1111),   .b_wren(m_data_write), .b_rddata(m_wrdata));

    //////////////////////////////////////////////////////////////////////////
    // Cache tag RAM
    //
    // Tag layout
    //    [8] Valid
    //    [7] Dirty
    //    [6] -
    //  [5:0] Tag
    //////////////////////////////////////////////////////////////////////////
    reg s_tag_write, m_tag_write;

    wire [8:0] s_tag_wrdata  = {2'b11, 1'b0, s_addr[16:11]};
    wire [8:0] s_tag_rddata;
    wire       s_tag_valid   = s_tag_rddata[8];
    wire       s_tag_dirty   = s_tag_rddata[7];
    wire [5:0] s_tag         = s_tag_rddata[5:0];

    wire [8:0] m_tag_wrdata  = {2'b10, 1'b0, m_addr[16:11]};
    wire [8:0] m_tag_rddata;   // unused

    dpram_tag cache_tags(
        .a_clk(clk), .a_addr(s_addr[10:0]), .a_wrdata(s_tag_wrdata), .a_wren(s_tag_write), .a_rddata(s_tag_rddata),
        .b_clk(clk), .b_addr(m_addr[10:0]), .b_wrdata(m_tag_wrdata), .b_wren(m_tag_write), .b_rddata(m_tag_rddata));

    //////////////////////////////////////////////////////////////////////////
    // Slave side state machine
    //////////////////////////////////////////////////////////////////////////
    localparam [2:0]
        SStIdle          = 3'd0,
        SStCheckTag      = 3'd1,
        SStWaitWriteback = 3'd2,
        SStWaitLoad      = 3'd3,
        SStWait          = 3'd4;

    reg  [2:0] ds_state,   qs_state;
    reg [16:0] dm_addr,    qm_addr;
    reg        dm_wren,    qm_wren;
    reg        dm_strobe,  qm_strobe;

    assign m_addr   = qm_addr;
    assign m_wren   = qm_wren;
    assign m_strobe = qm_strobe;

    always @* begin
        ds_state     = qs_state;
        dm_addr      = qm_addr;
        dm_wren      = qm_wren;
        dm_strobe    = qm_strobe;
        s_data_write = 0;
        s_tag_write  = 0;
        m_data_write = 0;
        m_tag_write  = 0;

        case (qs_state)
            SStIdle: begin
                if (s_strobe) begin
                    ds_state = SStCheckTag;
                end
            end

            SStCheckTag: begin
                if (s_tag_valid && s_tag == s_addr[16:11]) begin
                    // Cache line valid, perform read/write on cache line
                    ds_state     = SStIdle;
                    s_tag_write  = s_wren;
                    s_data_write = s_wren;

                end else begin
                    // Cache line does not match, fetch it from memory
                    if (s_tag_dirty) begin
                        // Perform writeback
                        dm_addr   = {s_tag, s_addr[10:0]};
                        dm_wren   = 1;
                        dm_strobe = 1;
                        ds_state  = SStWaitWriteback;

                    end else begin
                        // Fetch
                        dm_addr   = s_addr;
                        dm_wren   = 0;
                        dm_strobe = 1;
                        ds_state  = SStWaitLoad;
                    end
                end
            end

            SStWaitWriteback: begin
                if (!m_wait) begin
                    // Fetch
                    dm_addr   = s_addr;
                    dm_wren   = 0;
                    dm_strobe = 1;
                    ds_state  = SStWaitLoad;
                end
            end

            SStWaitLoad: begin
                if (!m_wait) begin
                    m_data_write = 1;
                    m_tag_write  = 1;
                    dm_strobe    = 0;
                    ds_state     = SStWait;
                end
            end

            SStWait: begin
                if (s_tag_valid && s_tag == s_addr[16:11]) begin
                    ds_state     = SStIdle;
                    s_tag_write  = s_wren;
                    s_data_write = s_wren;
                end
            end

            default: begin end
        endcase
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            qs_state  <= SStIdle;
            qm_addr   <= 0;
            qm_wren   <= 0;
            qm_strobe <= 0;
        end else begin
            qs_state  <= ds_state;
            qm_addr   <= dm_addr;
            qm_wren   <= dm_wren;
            qm_strobe <= dm_strobe;
        end

    assign s_wait = (ds_state != SStIdle);

endmodule
