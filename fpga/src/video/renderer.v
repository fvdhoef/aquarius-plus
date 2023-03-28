module renderer(
    input  wire        clk,
    input  wire        reset,

    // Data interface
    input  wire  [8:0] render_idx,
    input  wire [15:0] render_data,
    input  wire        render_start,
    output wire        last_pixel,
    output wire        busy,

    // Line buffer interface
    output wire  [8:0] wridx,
    output wire  [5:0] wrdata,
    output wire        wren
);

    reg [15:0] render_data_r, render_data_next;
    reg  [8:0] wridx_r,       wridx_next;
    reg  [5:0] wrdata_r,      wrdata_next;
    reg        wren_r,        wren_next;
    reg  [1:0] datasel_r,     datasel_next;
    reg        busy_r,        busy_next;
    reg        last_pixel_r,  last_pixel_next;

    assign wridx      = wridx_r;
    assign wrdata     = wrdata_r;
    assign wren       = wren_r;
    assign busy       = busy_r;
    assign last_pixel = last_pixel_r;

    always @* begin
        if (reset) begin
            render_data_next = 16'b0;
            wridx_next       = 9'd511;
            wrdata_next      = 6'b0;
            wren_next        = 1'b0;
            datasel_next     = 2'b0;
            busy_next        = 1'b0;
            last_pixel_next  = 1'b0;

        end else begin
            render_data_next = render_data_r;
            wridx_next       = wridx_r;
            wrdata_next      = wrdata_r;
            wren_next        = 1'b0;
            datasel_next     = datasel_r;
            busy_next        = busy_r;
            last_pixel_next  = 1'b0;

            if (render_start) begin
                render_data_next = {render_data[7:0], render_data[15:8]};
                datasel_next = 2'b00;
                wren_next    = 1'b1;
                busy_next    = 1'b1;
                wridx_next   = render_idx;
            end else if (busy_r) begin
                datasel_next = datasel_r + 2'b01;
                wren_next    = 1'b1;
                wridx_next = wridx_r + 9'd1;

                if (datasel_r == 2'b11) begin
                    busy_next  = 1'b0;
                    wren_next  = 1'b0;
                end
                if (datasel_r == 2'b10) begin
                    last_pixel_next = 1'b1;
                end
            end

            case (datasel_next)
                2'b00: wrdata_next = render_data_next[15:12];
                2'b01: wrdata_next = render_data_next[11:8];
                2'b10: wrdata_next = render_data_next[7:4];
                2'b11: wrdata_next = render_data_next[3:0];
            endcase
        end
    end

    always @(posedge clk) begin
        render_data_r <= render_data_next;
        wridx_r       <= wridx_next;
        wrdata_r      <= wrdata_next;
        wren_r        <= wren_next;
        datasel_r     <= datasel_next;
        busy_r        <= busy_next;
        last_pixel_r  <= last_pixel_next;
    end

endmodule
