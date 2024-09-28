`default_nettype none
`timescale 1 ns / 1 ps

module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [8:0] render_idx,
    input  wire [31:0] render_data,
    input  wire        render_start,
    input  wire        is_sprite,
    input  wire        hflip,
    input  wire  [1:0] palette,
    input  wire        render_priority,
    output wire        last_pixel,
    output wire        busy,

    // Line buffer interface
    output wire  [8:0] wridx,
    output wire  [5:0] wrdata,
    output wire        wren
);

    reg [31:0] render_data_r, render_data_next;
    reg  [1:0] palette_r,     palette_next;
    reg  [8:0] wridx_r,       wridx_next;
    reg  [5:0] wrdata_r,      wrdata_next;
    reg        wren_r,        wren_next;
    reg  [2:0] datasel_r,     datasel_next;
    reg        busy_r,        busy_next;
    reg        last_pixel_r,  last_pixel_next;
    reg        is_sprite_r,   is_sprite_next;
    reg        hflip_r,       hflip_next;
    reg        priority_r,    priority_next;

    assign wridx      = wridx_r;
    assign wrdata     = wrdata_r;
    assign wren       = wren_r;
    assign busy       = busy_r;
    assign last_pixel = last_pixel_r;

    reg [3:0] pixel_data;
    always @* case (datasel_next ^ (hflip_next ? 3'b111 : 3'b000))
        3'b000: pixel_data = render_data_next[31:28];
        3'b001: pixel_data = render_data_next[27:24];
        3'b010: pixel_data = render_data_next[23:20];
        3'b011: pixel_data = render_data_next[19:16];
        3'b100: pixel_data = render_data_next[15:12];
        3'b101: pixel_data = render_data_next[11:8];
        3'b110: pixel_data = render_data_next[7:4];
        3'b111: pixel_data = render_data_next[3:0];
    endcase

    wire lab_priority;
    wire lab_wrdata = priority_r && (wrdata_r[3:0] != 4'd0);

    lineattrbuf lab(
        .clk(clk),
        .idx1(wridx_r),
        .wrdata1(lab_wrdata),
        .wren1(wren_r),

        .idx2(wridx_next),
        .rddata2(lab_priority));

    always @* begin
        render_data_next = render_data_r;
        palette_next     = palette_r;
        wridx_next       = wridx_r;
        wrdata_next      = wrdata_r;
        wren_next        = 1'b0;
        datasel_next     = datasel_r;
        busy_next        = busy_r;
        last_pixel_next  = 1'b0;
        is_sprite_next   = is_sprite_r;
        hflip_next       = hflip_r;
        priority_next    = priority_r;

        if (render_start) begin
            render_data_next = render_data;
            palette_next     = palette;
            datasel_next     = 3'b000;
            wren_next        = 1'b1;
            busy_next        = 1'b1;
            wridx_next       = render_idx;
            is_sprite_next   = is_sprite;
            hflip_next       = hflip;
            priority_next    = render_priority;

        end else if (busy_r) begin
            datasel_next = datasel_r + 3'd1;
            wren_next    = 1'b1;
            wridx_next   = wridx_r + 9'd1;

            if (datasel_r == 3'd7) begin
                busy_next = 1'b0;
                wren_next = 1'b0;
            end
            if (datasel_r == 3'd6) begin
                last_pixel_next = 1'b1;
            end
        end

        wrdata_next[5:4] = palette_next[1:0];
        wrdata_next[3:0] = pixel_data;

        // Don't render transparent sprite pixels
        if (is_sprite_next) begin
            if (pixel_data == 4'd0 || (lab_priority && !priority_r))
                wren_next = 1'b0;
        end
    end

    always @(posedge clk) begin
        if (reset) begin
            render_data_r <= 32'b0;
            palette_r     <= 2'b0;
            wridx_r       <= 9'd511;
            wrdata_r      <= 6'b0;
            wren_r        <= 1'b0;
            datasel_r     <= 3'b0;
            busy_r        <= 1'b0;
            last_pixel_r  <= 1'b0;
            is_sprite_r   <= 1'b0;
            hflip_r       <= 1'b0;
            priority_r    <= 1'b0;

        end else begin
            render_data_r <= render_data_next;
            palette_r     <= palette_next;
            wridx_r       <= wridx_next;
            wrdata_r      <= wrdata_next;
            wren_r        <= wren_next;
            datasel_r     <= datasel_next;
            busy_r        <= busy_next;
            last_pixel_r  <= last_pixel_next;
            is_sprite_r   <= is_sprite_next;
            hflip_r       <= hflip_next;
            priority_r    <= priority_next;
        end
    end

endmodule
