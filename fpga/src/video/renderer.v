module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [8:0] render_idx,
    input  wire [31:0] render_data,
    input  wire        render_start,
    input  wire        hflip,
    input  wire  [1:0] palette,
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
    reg        hflip_r,       hflip_next;

    assign wridx      = wridx_r;
    assign wrdata     = wrdata_r;
    assign wren       = wren_r;
    assign busy       = busy_r;
    assign last_pixel = last_pixel_r;

    always @* begin
        if (reset) begin
            render_data_next = 32'b0;
            palette_next     = 2'b0;
            wridx_next       = 9'd511;
            wrdata_next      = 6'b0;
            wren_next        = 1'b0;
            datasel_next     = 2'b0;
            busy_next        = 1'b0;
            last_pixel_next  = 1'b0;
            hflip_next       = 1'b0;

        end else begin
            render_data_next = render_data_r;
            palette_next     = palette_r;
            wridx_next       = wridx_r;
            wrdata_next      = wrdata_r;
            wren_next        = 1'b0;
            datasel_next     = datasel_r;
            busy_next        = busy_r;
            last_pixel_next  = 1'b0;
            hflip_next       = hflip_r;

            if (render_start) begin
                render_data_next = render_data;
                palette_next     = palette;
                datasel_next     = 2'b00;
                wren_next        = 1'b1;
                busy_next        = 1'b1;
                wridx_next       = render_idx;
                hflip_next       = hflip;

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
            case (datasel_next ^ (hflip_next ? 3'b111 : 3'b000))
                3'b000: wrdata_next[3:0] = render_data_next[31:28];
                3'b001: wrdata_next[3:0] = render_data_next[27:24];
                3'b010: wrdata_next[3:0] = render_data_next[23:20];
                3'b011: wrdata_next[3:0] = render_data_next[19:16];
                3'b100: wrdata_next[3:0] = render_data_next[15:12];
                3'b101: wrdata_next[3:0] = render_data_next[11:8];
                3'b110: wrdata_next[3:0] = render_data_next[7:4];
                3'b111: wrdata_next[3:0] = render_data_next[3:0];
            endcase
        end
    end

    always @(posedge clk) begin
        render_data_r <= render_data_next;
        palette_r     <= palette_next;
        wridx_r       <= wridx_next;
        wrdata_r      <= wrdata_next;
        wren_r        <= wren_next;
        datasel_r     <= datasel_next;
        busy_r        <= busy_next;
        last_pixel_r  <= last_pixel_next;
        hflip_r       <= hflip_next;
    end

endmodule
