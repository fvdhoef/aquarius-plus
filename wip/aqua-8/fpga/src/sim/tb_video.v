`default_nettype none
`timescale 1ns / 1ps

module tb_video;

    reg clk   = 0;
    reg reset = 1;

    always #20 clk = !clk;
    always #200 reset = 0;

    //////////////////////////////////////////////////////////////////////////
    // VGA timing
    //////////////////////////////////////////////////////////////////////////
    reg [9:0] q_hcnt = 0;
    reg [9:0] q_vcnt = 0;

    wire hlast = (q_hcnt == 10'd799);
    wire vlast = (q_vcnt == 10'd524);

    always @(posedge(clk)) begin
        if (hlast) q_hcnt <= 0;
        else       q_hcnt <= q_hcnt + 10'd1;

        if (hlast) begin
            if (vlast) q_vcnt <= 0;
            else       q_vcnt <= q_vcnt + 10'd1;
        end
    end

    wire [9:0] hpos   = q_hcnt;
    wire [9:0] vpos10 = q_vcnt;

    wire hsync  = !(q_hcnt >= 10'd656 && q_hcnt <= 10'd751);
    wire vsync  = !(q_vcnt >= 10'd490 && q_vcnt <= 10'd491);

    wire hblank = !(q_hcnt < 10'd640);
    wire vblank = !(q_vcnt < 10'd480);

    wire  [9:0] video_hpos  = q_hcnt;
    wire        video_hlast = hlast;
    wire  [9:0] video_vpos  = q_vcnt;
    wire        video_vlast = vlast;


    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////
    wire irq_vblank;

    // Video RAM interface
    wire [12:0] vram_addr = 0;
    wire [31:0] vram_rddata;
    wire [31:0] vram_wrdata = 0;
    wire  [3:0] vram_wrsel = 0;
    wire        vram_wren = 0;

    // Text RAM interface
    wire [10:0] tram_addr = 0;
    wire [31:0] tram_rddata;
    wire [31:0] tram_wrdata = 0;
    wire  [3:0] tram_bytesel = 0;
    wire        tram_wren = 0;

    // Char RAM interface
    wire [10:0] chram_addr = 0;
    wire  [7:0] chram_rddata;
    wire  [7:0] chram_wrdata = 0;
    wire        chram_wren = 0;

    // Palette RAM interface
    wire  [3:0] pal_addr = 0;
    wire [11:0] pal_rddata;
    wire [11:0] pal_wrdata = 0;
    wire        pal_wren = 0;


    wire  [3:0] video_r;
    wire  [3:0] video_g;
    wire  [3:0] video_b;

    video video(
        .clk(clk),
        .reset(reset),

        .irq_vblank(irq_vblank),

        // Video RAM interface
        .vram_addr(vram_addr),
        .vram_rddata(vram_rddata),
        .vram_wrdata(vram_wrdata),
        .vram_wrsel(vram_wrsel),
        .vram_wren(vram_wren),

        // Text RAM interface
        .tram_addr(tram_addr),
        .tram_rddata(tram_rddata),
        .tram_wrdata(tram_wrdata),
        .tram_bytesel(tram_bytesel),
        .tram_wren(tram_wren),

        // Char RAM interface
        .chram_addr(chram_addr),
        .chram_rddata(chram_rddata),
        .chram_wrdata(chram_wrdata),
        .chram_wren(chram_wren),

        // Palette RAM interface
        .pal_addr(pal_addr),
        .pal_rddata(pal_rddata),
        .pal_wrdata(pal_wrdata),
        .pal_wren(pal_wren),

        // VGA output
        .video_hpos(video_hpos),
        .video_hlast(video_hlast),
        .video_vpos(video_vpos),
        .video_vlast(video_vlast),
        .video_r(video_r),
        .video_g(video_g),
        .video_b(video_b)
    );

endmodule
