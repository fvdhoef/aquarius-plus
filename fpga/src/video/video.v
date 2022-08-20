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
    // Horizontal counter
    //////////////////////////////////////////////////////////////////////////
    reg [8:0] hcnt_r = 9'd0;

    wire hactive   = (hcnt_r < 9'd352);
    wire hborder   = (hcnt_r < 9'd16 || hcnt_r >= 9'd336);
    wire hsync     = !(hcnt_r >= 9'd373 && hcnt_r < 9'd427);
    wire hcnt_done = (hcnt_r == 9'd454);

    always @(posedge(clk)) hcnt_r    <= hcnt_done ? 9'd0 : (hcnt_r + 9'd1);
    always @(posedge(clk)) vga_hsync <= hsync;

    //////////////////////////////////////////////////////////////////////////
    // Vertical counter
    //////////////////////////////////////////////////////////////////////////
    reg  [9:0] vcnt_r = 10'd0;
    wire [8:0] vcnt = vcnt_r[9:1];

    wire vactive   = (vcnt < 9'd240);
    wire vborder   = (vcnt < 9'd16 || vcnt >= 9'd216);
    wire vsync     = !(vcnt == 9'd245);
    wire vcnt_done = hcnt_done && vcnt_r == 10'd524;

    always @(posedge(clk))
        if (vcnt_done)
            vcnt_r <= 10'd0;
        else if (hcnt_done)
            vcnt_r <= vcnt_r + 10'd1;

    always @(posedge(clk)) vga_vsync <= vsync;

    //////////////////////////////////////////////////////////////////////////
    // Text address counter
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] row_addr_r;
    reg [9:0] text_addr_r;

    wire       next_row = vcnt_r[3:0] == 4'd15;
    wire [9:0] row_addr_next = row_addr_r + 10'd40;

    always @(posedge(clk))
        if (vborder)
            row_addr_r <= 10'd0;
        else if (hcnt_done && next_row)
            row_addr_r <= row_addr_next;

    always @(posedge(clk))
        if (hcnt_done)
            text_addr_r <= next_row ? row_addr_next : row_addr_r;
        else if (!hborder && hcnt_r[2:0] == 3'd7)
            text_addr_r <= text_addr_r + 10'd1;

    wire [9:0] text_addr = (!hactive || !vactive || hborder || vborder) ? 10'd0 : text_addr_r;

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

        .addr2(text_addr),
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

        .addr2(text_addr),
        .rddata2(color_data));

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] char_addr = {text_data, vcnt[2:0]};
    wire  [7:0] char_data;

    charram charram(
        .clk(clk),

        // .addr1(11'b0),
        // .rddata1(),
        // .wrdata1(8'h0),
        // .wren1(1'b0),

        .addr2(char_addr),
        .rddata2(char_data));

    reg [2:0] pixel_sel;
    always @(posedge(clk)) pixel_sel <= hcnt_r[2:0] ^ 3'b111;

    wire char_pixel = char_data[pixel_sel];
    wire [3:0] pixel_colidx = char_pixel ? color_data[7:4] : color_data[3:0];

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
        if (!hactive || !vactive) begin
            // Blank
            vga_r <= 4'b0;
            vga_g <= 4'b0;
            vga_b <= 4'b0;

        end else if (hborder || vborder) begin
            // Border color
            vga_r <= 4'h0;
            vga_g <= 4'hC;
            vga_b <= 4'hD;

        end else begin
            // Character pixel color
            vga_r <= pix_color[11:8];
            vga_g <= pix_color[7:4];
            vga_b <= pix_color[3:0];
        end

endmodule
