`default_nettype none
`timescale 1 ns / 1 ps

module video(
    input  wire         clk,
    input  wire         reset,

    // Palette interface
    input  wire   [7:0] palette_idx,
    input  wire  [11:0] palette_wrdata,
    input  wire         palette_wren,
    output wire  [11:0] palette_rddata,

    // Register interface
    input  wire         video_mode, // 0:320x240@8bpp, 1:640x240@4bpp
    input  wire  [18:0] base_addr,

    input  wire  [18:0] next_line_addr_wrdata,
    input  wire         next_line_addr_wren,
    output wire  [18:0] next_line_addr_rddata,

    // Memory interface (pipelined)
    output reg   [18:0] mem_addr,
    output reg          mem_strobe,
    input  wire         mem_wait,
    input  wire   [7:0] mem_rddata,
    input  wire         mem_rddata_valid,

    // Video output
    input  wire   [9:0] video_hpos,
    input  wire         video_hlast,
    input  wire   [9:0] video_vpos,
    input  wire         video_vlast,
    output reg    [3:0] video_r,
    output reg    [3:0] video_g,
    output reg    [3:0] video_b
);

    //////////////////////////////////////////////////////////////////////////
    // Memory bus mastering
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] linebuf_wrdata = mem_rddata;
    wire       linebuf_wren   = mem_rddata_valid;
    wire [8:0] vpos           = video_vpos[9:1];
    wire       line_end       = video_hlast && video_vpos[0];
    wire       start_fetch    = line_end && (vpos < 9'd239 || vpos == 9'd261);

    // Generate line buffer write index
    reg  [8:0] q_linebuf_wridx = 0;

    always @(posedge clk)
        if (line_end)              q_linebuf_wridx <= 0;
        else if (mem_rddata_valid) q_linebuf_wridx <= q_linebuf_wridx + 9'd1;

    // Generate memory addresses
    reg [18:0] q_mem_addr       = 0;
    reg [18:0] q_next_line_addr = 0;
    reg  [8:0] q_fetch_cnt      = 0;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            mem_strobe       <= 0;
            mem_addr         <= 0;
            q_mem_addr       <= 0;
            q_fetch_cnt      <= 0;
            q_next_line_addr <= 0;

        end else begin
            mem_strobe <= 0;

            // Fetch bytes
            if (q_fetch_cnt != 9'd0 && !mem_wait) begin
                mem_addr    <= q_mem_addr;
                mem_strobe  <= 1;
                q_mem_addr  <= q_mem_addr + 19'd1;
                q_fetch_cnt <= q_fetch_cnt - 9'd1;
            end

            // Start fetching of line?
            if (start_fetch) begin
                q_mem_addr       <= q_next_line_addr;
                q_next_line_addr <= q_next_line_addr + 19'd320;
                q_fetch_cnt      <= 9'd320;
            end

            // Set next line address to base address at end of frame
            if (line_end && vpos == 9'd239) q_next_line_addr <= base_addr;

            // Writing of next line register from CPU
            if (next_line_addr_wren) q_next_line_addr <= next_line_addr_wrdata;
        end
    end

    assign next_line_addr_rddata = q_next_line_addr;

    //////////////////////////////////////////////////////////////////////////
    // Line buffer
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] linebuf_rddata;

    reg q_hpos0;
    always @(posedge clk) q_hpos0 <= video_hpos[0];

    linebuf linebuf(
        .clk(clk),

        .linesel(video_vpos[1]),

        .idx1(q_linebuf_wridx),
        .wrdata1(linebuf_wrdata),
        .wren1(linebuf_wren),

        .idx2(video_hpos[9:1]),
        .rddata2(linebuf_rddata));

    wire [7:0] color_idx = video_mode
        ? {4'b0, q_hpos0 ? linebuf_rddata[7:4] : linebuf_rddata[3:0]}
        : linebuf_rddata;

    wire [11:0] color_data;

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    distram256d #(.WIDTH(12)) palette(
        .clk(clk),
        .a_addr(palette_idx),
        .a_rddata(palette_rddata),
        .a_wrdata(palette_wrdata),
        .a_wren({12{palette_wren}}),

        .b_addr(color_idx),
        .b_rddata(color_data)
    );

    always @(posedge clk) begin
        video_r <= {color_idx[7:5], color_idx[7]};  //  color_data[11:8];
        video_g <= {color_idx[4:2], color_idx[4]};  //  color_data[7:4];
        video_b <= {color_idx[1:0], color_idx[1:0]};  //  color_data[3:0];
    end

endmodule
