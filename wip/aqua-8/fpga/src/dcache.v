`default_nettype none
`timescale 1 ns / 1 ps

module dcache(
    input  wire        clk,
    input  wire        reset,

    // Slave bus interface (from CPU)
    input  wire [16:0] s_addr,
    input  wire [31:0] s_wrdata,
    input  wire  [3:0] s_bytesel,
    input  wire        s_wren,
    input  wire        s_flush,
    input  wire        s_strobe,
    output reg         s_wait,
    output reg  [31:0] s_rddata,

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
    // Cache line size is 1 word (4 bytes), so SRAM is divided in 131072 lines.
    // Cache size 8KB (2048 cache lines)
    //
    // s_addr:
    // [10: 0] -> (11 bits) word within line
    // [16:11] ->  (6 bits) tag

    //////////////////////////////////////////////////////////////////////////
    // Cache data and tag RAM
    //
    // Tag layout
    //     [40] Valid
    //     [39] Dirty
    //     [38] -
    //  [37:32] Tag (s_addr[16:11])
    //////////////////////////////////////////////////////////////////////////
    wire  [5:0] s_tag = s_addr[16:11];

    reg  [40:0] cram_wrdata;
    reg   [3:0] cram_bytesel;
    reg         cram_wren;
    wire [40:0] cram_rddata;

    wire        c_tag_valid      = cram_rddata[40];
    wire        c_tag_dirty      = cram_rddata[39];
    wire  [5:0] c_tag            = cram_rddata[37:32];
    wire        cache_line_valid = (c_tag_valid && c_tag == s_tag);
    wire        needs_wb         = c_tag_valid && c_tag_dirty;

    cache_tags cache_tags(.clk(clk), .addr(s_addr[10:0]), .wrdata(cram_wrdata[40:32]),                      .wren(cram_wren), .rddata(cram_rddata[40:32]));
    cache_ram  cache_ram (.clk(clk), .addr(s_addr[10:0]), .wrdata(cram_wrdata[31:0]), .wrsel(cram_bytesel), .wren(cram_wren), .rddata(cram_rddata[31: 0]));

    //////////////////////////////////////////////////////////////////////////
    // Slave side state machine
    //////////////////////////////////////////////////////////////////////////
    localparam [1:0]
        StIdle          = 2'd0,
        StCheckTag      = 2'd1,
        StWaitWriteback = 2'd2,
        StWaitLoad      = 2'd3;

    reg  [1:0] d_state,   q_state;
    reg [16:0] dm_addr,   qm_addr;
    reg        dm_wren,   qm_wren;
    reg        dm_strobe, qm_strobe;

    assign m_addr   = dm_addr;
    assign m_wren   = dm_wren;
    assign m_strobe = dm_strobe;

    reg do_access;
    reg do_fetch;

    wire wr_without_fetch = (!needs_wb && s_wren && s_bytesel == 4'b1111);

    assign m_wrdata = cram_rddata[31:0];

    always @* begin
        cram_wrdata  = {(s_flush ? 2'b00 : 2'b11), 1'b0, s_addr[16:11], s_wrdata};
        cram_bytesel = s_bytesel;
        cram_wren    = 0;
        s_rddata     = cram_rddata[31:0];
        s_wait       = 1;

        d_state      = q_state;
        dm_addr      = qm_addr;
        dm_wren      = qm_wren;
        dm_strobe    = qm_strobe;

        do_access    = 0;
        do_fetch     = 0;

        case (q_state)
            StIdle: begin
                if (s_strobe)
                    d_state = StCheckTag;
            end

            StCheckTag: begin
                if (s_flush) begin
                    if (!cache_line_valid) begin
                        s_wait    = 0;
                        d_state   = StIdle;
                    end else if (needs_wb) begin
                        // Perform writeback
                        dm_addr   = {c_tag, s_addr[10:0]};
                        dm_wren   = 1;
                        dm_strobe = 1;
                        d_state   = StWaitWriteback;

                    end else begin
                        cram_wren = 1;
                        s_wait    = 0;
                        d_state   = StIdle;
                    end
                    
                end else if (cache_line_valid || wr_without_fetch) begin
                    // Cache line valid, perform read/write on cache line
                    do_access = 1;

                end else if (needs_wb) begin
                    // Perform writeback
                    dm_addr   = {c_tag, s_addr[10:0]};
                    dm_wren   = 1;
                    dm_strobe = 1;
                    d_state   = StWaitWriteback;

                end else begin
                    // Non-dirty cache line does not match, fetch it from memory
                    do_fetch = 1;
                end
            end

            StWaitWriteback: begin
                if (!m_wait) begin
                    dm_strobe = 0;

                    if (s_flush) begin
                        cram_wren = 1;
                        s_wait    = 0;
                        d_state   = StIdle;
                    end else if (wr_without_fetch) begin
                        do_access = 1;
                    end else begin
                        do_fetch = 1;
                    end
                end
            end

            StWaitLoad: begin
                cram_wrdata[40:39] = 2'b10;
                cram_wrdata[31:0]  = m_rddata;
                cram_bytesel       = 4'b1111;
                s_rddata           = m_rddata;

                if (s_wren) begin
                    // Combine data
                    cram_wrdata[40:38] = 3'b110;
                    if (s_bytesel[3]) cram_wrdata[31:24] = s_wrdata[31:24];
                    if (s_bytesel[2]) cram_wrdata[23:16] = s_wrdata[23:16];
                    if (s_bytesel[1]) cram_wrdata[15: 8] = s_wrdata[15: 8];
                    if (s_bytesel[0]) cram_wrdata[ 7: 0] = s_wrdata[ 7: 0];
                end

                if (!m_wait) begin
                    dm_strobe = 0;
                    cram_wren = 1;
                    s_wait    = 0;
                    d_state   = StIdle;
                end
            end

            default: begin end
        endcase

        if (do_access) begin
            cram_wren = s_wren;
            s_wait    = 0;
            d_state   = StIdle;

        end else if (do_fetch) begin
            // Fetch
            dm_addr   = s_addr;
            dm_wren   = 0;
            dm_strobe = 1;
            d_state   = StWaitLoad;
        end
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_state   <= StIdle;
            qm_addr   <= 0;
            qm_wren   <= 0;
            qm_strobe <= 0;
        end else begin
            q_state   <= d_state;
            qm_addr   <= dm_addr;
            qm_wren   <= dm_wren;
            qm_strobe <= dm_strobe;
        end

endmodule
