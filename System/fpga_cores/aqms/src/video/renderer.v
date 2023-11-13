module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [7:0] render_idx,
    input  wire [31:0] render_data,
    input  wire        render_start,
    input  wire        is_sprite,
    input  wire        hflip,
    input  wire        palette,
    input  wire        priority,
    output wire        last_pixel,
    output reg         spr_collision,
    output wire        busy,

    // Line buffer interface
    output wire  [7:0] wridx,
    output wire  [4:0] wrdata,
    output wire        wren
);

    reg [31:0] render_data_r, render_data_next;
    reg        palette_r,     palette_next;
    reg  [7:0] wridx_r,       wridx_next;
    reg  [4:0] wrdata_r,      wrdata_next;
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

    wire [1:0] lab_wrdata = {is_sprite_r, (priority_r && (wrdata[3:0] != 4'd0))};
    wire lab_is_sprite, lab_priority;

    lineattrbuf lab(
        .clk(clk),
        .idx1(wridx),
        .wrdata1(lab_wrdata),
        .wren1(wren),

        .idx2(wridx_next),
        .rddata2({lab_is_sprite, lab_priority}));

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
        spr_collision    = 1'b0;

        if (render_start) begin
            render_data_next = render_data;
            palette_next     = palette;
            datasel_next     = 2'b00;
            wren_next        = 1'b1;
            busy_next        = 1'b1;
            wridx_next       = render_idx;
            is_sprite_next   = is_sprite;
            hflip_next       = hflip;
            priority_next    = priority;

        end else if (busy_r) begin
            datasel_next = datasel_r + 3'd1;
            wren_next    = 1'b1;
            wridx_next   = wridx_r + 8'd1;

            if (datasel_r == 3'd7) begin
                busy_next = 1'b0;
                wren_next = 1'b0;
            end
            if (datasel_r == 3'd6) begin
                last_pixel_next = 1'b1;
            end
        end

        wrdata_next[4]   = palette_next;
        wrdata_next[3:0] = pixel_data;

        if (is_sprite_next) begin
            // Don't render transparent sprite pixels
            if (pixel_data == 4'd0 || lab_is_sprite || lab_priority)
                wren_next = 1'b0;

            // Check for collision
            if (pixel_data != 4'd0 && lab_is_sprite)
                spr_collision = 1'b1;
        end
    end

    always @(posedge clk) begin
        if (reset) begin
            render_data_r <= 32'b0;
            palette_r     <= 1'b0;
            wridx_r       <= 8'd0;
            wrdata_r      <= 6'b0;
            wren_r        <= 1'b0;
            datasel_r     <= 2'b0;
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
