`default_nettype none
`timescale 1 ns / 1 ps

module video(
    input  wire        clk,
    input  wire        reset,

    // Register interface
    input  wire        reg_bm_wrap,
    input  wire        reg_layer2_enable,
    input  wire        reg_sprites_enable,
    input  wire        reg_gfx_tilemode,
    input  wire        reg_gfx_enable,
    input  wire        reg_text_priority,
    input  wire        reg_text_mode80,
    input  wire        reg_text_enable,
    input  wire  [8:0] reg_layer1_scrx,
    input  wire  [7:0] reg_layer1_scry,
    input  wire  [8:0] reg_layer2_scrx,
    input  wire  [7:0] reg_layer2_scry,
    input  wire  [8:0] reg_irqline,
    output wire  [8:0] vline,

    output wire        irq_line,
    output wire        irq_vblank,

    // Sprite attribute interface
    input  wire  [8:0] sprattr_addr,
    output wire [31:0] sprattr_rddata,
    input  wire [31:0] sprattr_wrdata,
    input  wire        sprattr_wren,

    // Text RAM interface
    input  wire [10:0] tram_addr,
    output wire [31:0] tram_rddata,
    input  wire [31:0] tram_wrdata,
    input  wire  [3:0] tram_bytesel,
    input  wire        tram_wren,

    // Char RAM interface
    input  wire [10:0] chram_addr,
    output wire  [7:0] chram_rddata,
    input  wire  [7:0] chram_wrdata,
    input  wire        chram_wren,

    // Palette RAM interface
    input  wire  [6:0] pal_addr,
    output wire [11:0] pal_rddata,
    input  wire [11:0] pal_wrdata,
    input  wire        pal_wren,

    // Video RAM interface
    input  wire [12:0] vram_addr,
    input  wire [31:0] vram_wrdata,
    input  wire  [7:0] vram_wrsel,
    input  wire        vram_wren,
    output wire [31:0] vram_rddata,

    // VGA output
    output reg   [3:0] video_r,
    output reg   [3:0] video_g,
    output reg   [3:0] video_b,
    output reg         video_de,
    output reg         video_hsync,
    output reg         video_vsync,
    output wire        video_newframe,
    output reg         video_oddline);

    wire [8:0] vpos9;
    wire       vblank;

    assign vline = vpos9;

    reg q_vblank;
    always @(posedge clk) q_vblank <= vblank;

    wire [7:0] rddata_sprattr;

    wire irqline_match = (vline == reg_irqline);
    reg q_irqline_match;
    always @(posedge clk) q_irqline_match <= irqline_match;

    assign irq_line   = !q_irqline_match && irqline_match;
    assign irq_vblank = !q_vblank        && vblank;

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [9:0] hpos;
    wire       hsync, hblank, hlast;
    wire [9:0] vpos10;
    wire       vsync, vnext;
    wire       blank;

    aqp_video_timing video_timing(
        .clk(clk),
        .mode(1'b1),

        .hpos(hpos),
        .hsync(hsync),
        .hblank(hblank),
        .hlast(hlast),

        .vpos(vpos10),
        .vsync(vsync),
        .vblank(vblank),
        .vnext(vnext),
        .vnewframe(video_newframe),

        .blank(blank));

    always @(posedge clk) video_oddline <= vpos10[0];

    assign vpos9 = vpos10[9:1];

    reg [9:0] q_hpos, q2_hpos;
    always @(posedge clk) q_hpos  <= hpos;
    always @(posedge clk) q2_hpos <= q_hpos;

    reg q_blank, q_hsync, q_vsync;
    always @(posedge clk) q_blank <= blank;
    always @(posedge clk) q_hsync <= hsync;
    always @(posedge clk) q_vsync <= vsync;

    reg q2_blank, q2_hsync, q2_vsync;
    always @(posedge clk) q2_blank <= q_blank;
    always @(posedge clk) q2_hsync <= q_hsync;
    always @(posedge clk) q2_vsync <= q_vsync;

    //////////////////////////////////////////////////////////////////////////
    // Character address
    //////////////////////////////////////////////////////////////////////////
    reg         q_mode80         = 1'b0;
    reg  [11:0] q_row_addr       = 12'd0;
    reg  [11:0] q_char_addr      = 12'd0;
    wire        next_row         = vnext && (vpos9[2:0] == 3'd7);
    wire [11:0] d_row_addr       = q_row_addr + (q_mode80 ? 12'd80 : 12'd40);
    wire [11:0] border_char_addr = 12'h7FF;

    always @(posedge(clk))
        if (vblank) begin
            q_mode80   <= reg_text_mode80;
            q_row_addr <= 12'd0;
        end else if (next_row) begin
            q_row_addr <= d_row_addr;
        end

    wire next_char    = q_mode80 ? (hpos[2:0] == 3'd0) : (hpos[3:0] == 4'd0);
    wire start_active = q_blank && !blank;

    reg  [11:0] d_char_addr;
    always @* begin
        if      (start_active) d_char_addr = q_row_addr;
        else if (next_char)    d_char_addr = q_char_addr + 12'd1;
        else                   d_char_addr = q_char_addr;
    end

    always @(posedge(clk)) q_char_addr <= d_char_addr;

    //////////////////////////////////////////////////////////////////////////
    // Text RAM
    //////////////////////////////////////////////////////////////////////////
    wire [31:0] textram_rddata;

    dpram8k textram(
        .a_clk(clk),
        .a_addr(tram_addr),
        .a_wrdata(tram_wrdata),
        .a_wrsel(tram_bytesel), 
        .a_wren(tram_wren),
        .a_rddata(tram_rddata),

        .b_clk(clk),
        .b_addr(d_char_addr[11:1]),
        .b_wrdata(32'b0),
        .b_wrsel(4'b0), 
        .b_wren(1'b0),
        .b_rddata(textram_rddata));

    wire [15:0] textram_color_text = q_char_addr[0] ? textram_rddata[31:16] : textram_rddata[15:0];
    wire  [7:0] text_data  = textram_color_text[7:0];
    wire  [7:0] color_data = textram_color_text[15:8];

    reg [7:0] q_color_data;
    always @(posedge clk) q_color_data <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos9[2:0]};
    wire  [7:0] charram_data;

    charram charram(
        .clk1(clk),
        .addr1(chram_addr),
        .rddata1(chram_rddata),
        .wrdata1(chram_wrdata),
        .wren1(chram_wren),

        .clk2(clk),
        .addr2(charram_addr),
        .rddata2(charram_data));

    wire [2:0] pixel_sel    = (q_mode80 ? q2_hpos[2:0] : q2_hpos[3:1]) ^ 3'b111;
    wire       char_pixel   = charram_data[pixel_sel];
    wire [3:0] text_colidx  = char_pixel ? q_color_data[7:4] : q_color_data[3:0];

    //////////////////////////////////////////////////////////////////////////
    // Sprite attribute RAM
    //////////////////////////////////////////////////////////////////////////
    wire  [7:0] spr_sel;
    wire  [8:0] spr_x;
    wire  [7:0] spr_y;
    wire  [9:0] spr_idx;
    wire  [1:0] spr_zdepth;
    wire  [2:0] spr_palette;
    wire        spr_h16;
    wire        spr_vflip;
    wire        spr_hflip;

    localparam SPRATTR_BITS = 35;

    wire [(SPRATTR_BITS-1):0] sprattr_a_rddata;
    wire [(SPRATTR_BITS-1):0] sprattr_a_wrdata = {
        sprattr_wrdata[17:0],    // Attributes
        sprattr_wrdata[23:16],   // Y
        sprattr_wrdata[8:0]      // X
    };
    wire [(SPRATTR_BITS-1):0] sprattr_a_wren = {
        {18{sprattr_wren && !sprattr_addr[8]}},
        {17{sprattr_wren &&  sprattr_addr[8]}}
    };
    assign sprattr_rddata = !sprattr_addr[8] ?
        {16'b0, sprattr_a_rddata[32:17]} :
        {8'b0, sprattr_a_rddata[16:9], 7'b0, sprattr_a_rddata[8:0]};

    distram256d #(.WIDTH(SPRATTR_BITS)) sprattr(
        .clk(clk),
        .a_addr(sprattr_addr[7:0]),
        .a_rddata(sprattr_a_rddata),
        .a_wrdata(sprattr_a_wrdata),
        .a_wren(sprattr_a_wren),
        .b_addr(spr_sel),
        .b_rddata({
            spr_zdepth,
            spr_palette,
            spr_vflip,
            spr_hflip,
            spr_h16,
            spr_idx,
            spr_y,
            spr_x
        }));

    //////////////////////////////////////////////////////////////////////////
    // VRAM
    //////////////////////////////////////////////////////////////////////////
    wire [13:0] vram_addr2;
    wire [31:0] vram_rddata2_32;
    wire [15:0] vram_rddata2;

    reg q_vram_addr2_0;
    always @(posedge clk) q_vram_addr2_0 <= vram_addr2[0];

    dpram32k vram(
        .a_clk(clk),
        .a_addr(vram_addr),
        .a_wrdata(vram_wrdata),
        .a_wrsel(vram_wrsel),
        .a_wren(vram_wren),
        .a_rddata(vram_rddata),

        .b_clk(clk),
        .b_addr(vram_addr2[13:1]),
        .b_wrdata(32'b0),
        .b_wrsel(8'b0),
        .b_wren(1'b0),
        .b_rddata(vram_rddata2_32));

    assign vram_rddata2 = q_vram_addr2_0 ? vram_rddata2_32[31:16] : vram_rddata2_32[15:0];

    //////////////////////////////////////////////////////////////////////////
    // Graphics
    //////////////////////////////////////////////////////////////////////////
    wire [6:0] linebuf_data;
    reg  [8:0] q_linebuf_rdidx;

    always @(posedge clk) q_linebuf_rdidx <= hpos[9:1];

    reg q_gfx_start;
    always @(posedge clk) q_gfx_start <= vnext;

    reg  [7:0] gfx_vline;
    always @* begin
        if      (vpos9 == 9'd261) gfx_vline = 8'd0;
        else if (vpos9 == 9'd262) gfx_vline = 8'd1;
        else                      gfx_vline = vpos9[7:0] + 8'd1;
    end

    gfx gfx(
        .clk(clk),
        .reset(reset),

        // Register values
        .gfx_enable(reg_gfx_enable),
        .tilemode(reg_gfx_tilemode),
        .bm_wrap(reg_bm_wrap),
        .sprites_enable(reg_sprites_enable),
        .layer2_enable(reg_layer2_enable),
        .layer1_scrx(reg_layer1_scrx),
        .layer1_scry(reg_layer1_scry),
        .layer2_scrx(reg_layer2_scrx),
        .layer2_scry(reg_layer2_scry),

        // Sprite attribute interface
        .spr_sel(spr_sel),
        .spr_x(spr_x),
        .spr_y(spr_y),
        .spr_idx(spr_idx),
        .spr_zdepth(spr_zdepth),
        .spr_palette(spr_palette),
        .spr_h16(spr_h16),
        .spr_vflip(spr_vflip),
        .spr_hflip(spr_hflip),

        // Video RAM interface
        .vaddr(vram_addr2),
        .vdata(vram_rddata2),

        // Render parameters
        .vline(gfx_vline),
        .start(q_gfx_start),

        // Line buffer interface
        .linebuf_rdidx(q_linebuf_rdidx),
        .linebuf_data(linebuf_data));

    //////////////////////////////////////////////////////////////////////////
    // Compositing
    //////////////////////////////////////////////////////////////////////////
    reg  [6:0] pixel_colidx;

    always @* begin
        pixel_colidx = 7'b0;
        if (reg_text_enable && !reg_text_priority)
            pixel_colidx = {3'b0, text_colidx};
        if (!reg_text_enable || reg_text_priority || linebuf_data[3:0] != 4'd0)
            pixel_colidx = linebuf_data;
        if (reg_text_enable && reg_text_priority && text_colidx != 4'd0)
            pixel_colidx = {3'b0, text_colidx};
    end

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] pal_r, pal_g, pal_b;

    distram128d #(.WIDTH(12)) palette(
        .clk(clk),
        .a_addr(pal_addr),
        .a_rddata(pal_rddata),
        .a_wrdata(pal_wrdata),
        .a_wren({12{pal_wren}}),
        .b_addr(pixel_colidx),
        .b_rddata({pal_r, pal_g, pal_b}));

    //////////////////////////////////////////////////////////////////////////
    // Output registers
    //////////////////////////////////////////////////////////////////////////
    always @(posedge(clk))
        if (q2_blank) begin
            video_r  <= 4'b0;
            video_g  <= 4'b0;
            video_b  <= 4'b0;
            video_de <= 1'b0;

        end else begin
            video_r  <= pal_r;
            video_g  <= pal_g;
            video_b  <= pal_b;
            video_de <= 1'b1;
        end

    always @(posedge clk) video_hsync <= q2_hsync;
    always @(posedge clk) video_vsync <= q2_vsync;

endmodule
