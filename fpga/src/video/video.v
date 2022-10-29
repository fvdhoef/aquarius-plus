module video(
    input  wire        clk,
    input  wire        reset,

    // IO register interface
    input  wire  [3:0] io_addr,
    output reg   [7:0] io_rddata,
    input  wire  [7:0] io_wrdata,
    input  wire        io_wren,

    // Text RAM interface
    input  wire [10:0] tram_addr,
    output wire  [7:0] tram_rddata,
    input  wire  [7:0] tram_wrdata,
    input  wire        tram_wren,

    // Char RAM interface
    input  wire [10:0] chram_addr,
    output wire  [7:0] chram_rddata,
    input  wire  [7:0] chram_wrdata,
    input  wire        chram_wren,

    // Video RAM interface
    input  wire [13:0] vram_addr,
    output wire  [7:0] vram_rddata,
    input  wire  [7:0] vram_wrdata,
    input  wire        vram_wren,

    // VGA output
    output reg   [3:0] vga_r,
    output reg   [3:0] vga_g,
    output reg   [3:0] vga_b,
    output reg         vga_hsync,
    output reg         vga_vsync,
    
    output wire        vga_vblank);

    reg  [6:0] vpalsel_r;               // IO $EA
    wire [7:0] rddata_vpaldata;

    //////////////////////////////////////////////////////////////////////////
    // IO registers
    //////////////////////////////////////////////////////////////////////////
    wire sel_io_vpalsel  = (io_addr == 4'hA);
    wire sel_io_vpaldata = (io_addr == 4'hB);

    always @* begin
        io_rddata <= 8'h00;
        if (sel_io_vpalsel)  io_rddata <= {1'b0, vpalsel_r};    // IO $EA
        if (sel_io_vpaldata) io_rddata <= rddata_vpaldata;      // IO $EB
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            vpalsel_r <= 7'b0;
        end else begin
            if (io_wren && sel_io_vpalsel) vpalsel_r <= io_wrdata[6:0];
        end

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

    assign vga_vblank = vblank;

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

    wire       next_char = (hpos[2:0] == 3'd7);
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
    wire [15:0] textram_rddata;
    wire [10:0] p1_addr = {tram_addr[9:0], tram_addr[10]};

    textram textram(
        .p1_clk(clk),
        .p1_addr(p1_addr),
        .p1_rddata(tram_rddata),
        .p1_wrdata(tram_wrdata),
        .p1_wren(tram_wren),

        .p2_clk(clk),
        .p2_addr(char_addr_r),
        .p2_rddata(textram_rddata));

    wire [7:0] text_data  = textram_rddata[7:0];
    wire [7:0] color_data = textram_rddata[15:8];

    reg [7:0] color_data_r;
    always @(posedge clk) color_data_r <= color_data;

    //////////////////////////////////////////////////////////////////////////
    // Character RAM
    //////////////////////////////////////////////////////////////////////////
    wire [10:0] charram_addr = {text_data, vpos[2:0]};
    wire  [7:0] charram_data;

    charram charram(
        .clk(clk),

        .addr1(chram_addr),
        .rddata1(chram_rddata),
        .wrdata1(chram_wrdata),
        .wren1(chram_wren),

        .addr2(charram_addr),
        .rddata2(charram_data));

    wire [2:0] pixel_sel    = hpos_rr[2:0] ^ 3'b111;
    wire       char_pixel   = charram_data[pixel_sel];
    wire [3:0] pixel_colidx = char_pixel ? color_data_r[7:4] : color_data_r[3:0];

    //////////////////////////////////////////////////////////////////////////
    // VRAM
    //////////////////////////////////////////////////////////////////////////
    wire [13:0] vram_addr2 = 14'b0;

    vram vram(
        // First port - CPU access
        .p1_clk(clk),
        .p1_addr(vram_addr),
        .p1_rddata(vram_rddata),
        .p1_wrdata(vram_wrdata),
        .p1_wren(vram_wren),

        // Second port - Video access
        .p2_clk(clk),
        .p2_addr(vram_addr2),
        .p2_rddata());

    //////////////////////////////////////////////////////////////////////////
    // Palette
    //////////////////////////////////////////////////////////////////////////
    wire [11:0] pix_color;
    wire [11:0] palette_rddata;

    wire palram_wren_l = io_wren && sel_io_vpaldata && !vpalsel_r[0];
    wire palram_wren_h = io_wren && sel_io_vpaldata &&  vpalsel_r[0];

    parameter [16*12-1:0] PALETTE_INIT = {
        12'h111, 12'hF11, 12'h1F1, 12'hFF1, 12'h22E, 12'hF1F, 12'h3CC, 12'hFFF,
        12'hCCC, 12'h3BB, 12'hC2C, 12'h419, 12'hFF7, 12'h2D4, 12'hB22, 12'h333
    };

    generate
        genvar i;
        for (i=0; i<12; i=i+1) begin: palram_gen
            RAM16X1D #(
                .INIT({
                    PALETTE_INIT[ 0*12+i], PALETTE_INIT[ 1*12+i], PALETTE_INIT[ 2*12+i], PALETTE_INIT[ 3*12+i],
                    PALETTE_INIT[ 4*12+i], PALETTE_INIT[ 5*12+i], PALETTE_INIT[ 6*12+i], PALETTE_INIT[ 7*12+i],
                    PALETTE_INIT[ 8*12+i], PALETTE_INIT[ 9*12+i], PALETTE_INIT[10*12+i], PALETTE_INIT[11*12+i],
                    PALETTE_INIT[12*12+i], PALETTE_INIT[13*12+i], PALETTE_INIT[14*12+i], PALETTE_INIT[15*12+i]
                }))
            
            palram(
                .DPRA3(pixel_colidx[3]), .DPRA2(pixel_colidx[2]), .DPRA1(pixel_colidx[1]), .DPRA0(pixel_colidx[0]), .DPO(pix_color[i]),
                .A3(   vpalsel_r[4]),    .A2(   vpalsel_r[3]),    .A1(   vpalsel_r[2]),    .A0(   vpalsel_r[1]),    .SPO(palette_rddata[i]),
                .WCLK(clk), .D(io_wrdata[i & 7]), .WE((i<8) ? palram_wren_l : palram_wren_h));
        end
    endgenerate

    assign rddata_vpaldata = vpalsel_r[0] ? {4'h0, palette_rddata[11:8]} : palette_rddata[7:0];

    //////////////////////////////////////////////////////////////////////////
    // Output registers
    //////////////////////////////////////////////////////////////////////////
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
