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
    output wire  [3:0] vga_r,
    output wire  [3:0] vga_g,
    output wire  [3:0] vga_b,
    output wire        vga_hsync,
    output wire        vga_vsync);

    assign vga_r        = 4'b0;
    assign vga_g        = 4'b0;
    assign vga_b        = 4'b0;
    assign vga_hsync    = 1'b0;
    assign vga_vsync    = 1'b0;

endmodule
