module video(
    input  wire        clk,
    input  wire        reset,

    // VCTRL
    // input  wire        text_enable,
    // input  wire        layer_enable,
    // input  wire        layer_tile_mode,  // 0: bitmap mode, 1: tile mode 
    // input  wire        text_priority,

    // VSCRX_L/VSCRX_H/VSCRY
    // input  wire  [8:0] layer_scroll_x,
    // input  wire  [7:0] layer_scroll_y,

    // Sprite attribute RAM interface
    // output wire  [5:0] sprite_sel,
    // input  wire  [8:0] sprite_x,
    // input  wire  [7:0] sprite_y,
    // input  wire  [8:0] sprite_tile_idx,
    // input  wire        sprite_enable,
    // input  wire        sprite_priority,
    // input  wire  [1:0] sprite_palette,
    // input  wire        sprite_h16,
    // input  wire        sprite_vflip,
    // input  wire        sprite_hflip,

    // Text RAM interface
    // output wire  [9:0] tram_addr,
    // input  wire  [7:0] tram_char,
    // input  wire  [7:0] tram_color,

    // Video RAM interface
    // output wire [13:0] vram_addr,
    // input  wire  [7:0] vram_data,

    // Line buffer interface
    // output wire  [8:0] lbuf_idx,
    // output wire  [7:0] lbuf_wrdata,
    // input  wire  [7:0] lbuf_rddata,
    // output wire        lbuf_wren
    
    // VGA output
    output reg  [3:0] vga_r,
    output reg  [3:0] vga_g,
    output reg  [3:0] vga_b,
    output reg        vga_hsync,
    output reg        vga_vsync);

    //////////////////////////////////////////////////////////////////////////
    // Video timing
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] hpos;
    wire       hsync, hblank, hlast;
    wire [7:0] vpos;
    wire       vsync, vblank, vnext;
    wire       blank;

    video_timing video_timing(
        .clk(clk),

        .hpos(hpos),
        .hsync(hsync),
        .hblank(hblank),
        .hlast(hlast),
        
        .vpos(vpos),
        .vsync(vsync),
        .vblank(vblank),
        .vnext(vnext),
        
        .blank(blank));

    wire hborder = hpos < 9'd16 || hpos >= 9'd336;
    wire vborder = vpos < 9'd16 || vpos >= 9'd216;

    reg [8:0] hpos_r, hpos_rr;
    always @(posedge clk) hpos_r <= hpos;
    always @(posedge clk) hpos_rr <= hpos_r;

    reg blank_r, hsync_r, vsync_r;
    always @(posedge clk) blank_r <= blank;
    always @(posedge clk) hsync_r <= hsync;
    always @(posedge clk) vsync_r <= vsync;

    reg blank_rr, hsync_rr, vsync_rr;
    always @(posedge clk) blank_rr <= blank_r;
    always @(posedge clk) hsync_rr <= hsync_r;
    always @(posedge clk) vsync_rr <= vsync_r;

    //////////////////////////////////////////////////////////////////////////
    // Character address
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] row_addr_r = 10'd0;
    reg [9:0] char_addr_r = 10'd0;

    wire       next_row = (vpos >= 9'd23) && vnext && (vpos[2:0] == 3'd7);
    wire [9:0] row_addr_next = row_addr_r + 10'd40;

    always @(posedge(clk))
        if (vblank)
            row_addr_r <= 10'd0;
        else if (next_row)
            row_addr_r <= row_addr_next;

    wire next_char = (hpos[2:0] == 3'd7);
    wire [5:0] column = hpos[8:3];

    always @(posedge(clk))
        if (next_char) begin
            if (vborder || column == 6'd0 || column >= 6'd41)
                char_addr_r <= 10'd0;
            else if (column == 6'd1)
                char_addr_r <= row_addr_r;
            else
                char_addr_r <= char_addr_r + 10'd1;
        end

    //////////////////////////////////////////////////////////////////////////
    // Text RAM
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] text_data;
    
    textram textram(
        .clk(clk),

        // input  wire [10:0] addr1,
        // output reg   [7:0] rddata1,
        // input  wire  [7:0] wrdata1,
        // input  wire        wren1,

        .addr2(char_addr_r),
        .rddata2(text_data));

    //////////////////////////////////////////////////////////////////////////
    // Color RAM
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] color_data;
    
    colorram colorram(
        .clk(clk),

        // input  wire [10:0] addr1,
        // output reg   [7:0] rddata1,
        // input  wire  [7:0] wrdata1,
        // input  wire        wren1,

        .addr2(char_addr_r),
        .rddata2(color_data));

    reg [7:0] color_data_r;
    always @(posedge clk) color_data_r <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    wire  [7:0] charram_data;

    charram charram(
        .clk(clk),

        // .addr1(11'b0),
        // .rddata1(),
        // .wrdata1(8'h0),
        // .wren1(1'b0),

        .addr2(charram_addr),
        .rddata2(charram_data));

    wire [2:0] pixel_sel = hpos_rr[2:0] ^ 3'b111;

    wire char_pixel = charram_data[pixel_sel];
    wire [3:0] pixel_colidx = char_pixel ? color_data_r[7:4] : color_data_r[3:0];

    reg [11:0] pix_color;
    always @* case (pixel_colidx)
        4'h0: pix_color = 12'h111;
        4'h1: pix_color = 12'hF11;
        4'h2: pix_color = 12'h1F1;
        4'h3: pix_color = 12'hFF1;
        4'h4: pix_color = 12'h22E;
        4'h5: pix_color = 12'hF1F;
        4'h6: pix_color = 12'h3CC;
        4'h7: pix_color = 12'hFFF;
        4'h8: pix_color = 12'hCCC;
        4'h9: pix_color = 12'h3BB;
        4'hA: pix_color = 12'hC2C;
        4'hB: pix_color = 12'h419;
        4'hC: pix_color = 12'hFF7;
        4'hD: pix_color = 12'h2D4;
        4'hE: pix_color = 12'hB22;
        4'hF: pix_color = 12'h333;
    endcase

    always @(posedge(clk))
        if (blank_rr) begin
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else begin
            vga_r <= pix_color[11:8];
            vga_g <= pix_color[7:4];
            vga_b <= pix_color[3:0];
        end

    always @(posedge clk) vga_hsync <= hsync_rr;
    always @(posedge clk) vga_vsync <= vsync_rr;

endmodule
