module esp_uart_rx(
    input  wire        clk,
    input  wire        rst,

    input  wire        uart_rxd,

    output reg  [7:0]  rx_data,
    output reg         rx_valid);

    reg started_r;

    // Synchronize input signal
    reg [3:0] rxd_r;
    always @(posedge clk) rxd_r <= {rxd_r[2:0], uart_rxd};
    wire rx_in = rxd_r[2];
    wire start_condition = (rxd_r[3:2] == 'b10);

    // Bit-timing
    reg [2:0] clk_cnt_r;
    always @(posedge clk)
        if (!started_r)
            clk_cnt_r <= 0;
        else
            clk_cnt_r <= clk_cnt_r + 3'd1;

    // Receive logic
    reg [3:0] bit_cnt_r;
    reg [7:0] shift_r;
    always @(posedge clk or posedge rst)
        if (rst) begin
            started_r <= 0;
            rx_valid  <= 0;
            shift_r   <= 0;
            rx_data   <= 0;
            bit_cnt_r <= 0;

        end else begin
            rx_valid <= 0;

            if (!started_r) begin
                bit_cnt_r <= 4'd0;

                if (start_condition)
                    started_r <= 1;

            end else if (clk_cnt_r == 3'd4) begin
                shift_r   <= {rx_in, shift_r[7:1]};
                bit_cnt_r <= bit_cnt_r + 4'd1;

                if (bit_cnt_r == 4'd9) begin
                    started_r <= 0;

                    if (rx_in) begin
                        rx_data  <= shift_r;
                        rx_valid <= 1;
                    end
                end
            end
        end

endmodule
