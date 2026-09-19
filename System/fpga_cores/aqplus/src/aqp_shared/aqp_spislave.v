`default_nettype none
`timescale 1 ns / 1 ps

module aqp_spislave(
    input  wire        clk,

    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,

    output wire        msg_start,
    output wire        msg_end,
    output wire  [7:0] rxdata,
    output wire        rxdata_valid,

    input  wire  [7:0] txdata,
    output reg         txdata_ack);

    // Synchronize SCLK
    reg [2:0] q_sclk;
    always @(posedge clk) q_sclk <= {q_sclk[1:0], esp_sclk};
    wire sclk_rising  = (q_sclk[2:1] == 2'b01);
    wire sclk_falling = (q_sclk[2:1] == 2'b10);

    // Synchronize SSEL#
    reg [2:0] q_ssel_n;
    always @(posedge clk) q_ssel_n <= {q_ssel_n[1:0], esp_ssel_n};
    wire ssel_active    = ~q_ssel_n[1];
    wire ssel_msg_start = (q_ssel_n[2:1] == 2'b10);
    wire ssel_msg_end   = (q_ssel_n[2:1] == 2'b01);

    // Synchronize MOSI
    reg [1:0] q_mosi;
    always @(posedge clk) q_mosi <= {q_mosi[0], esp_mosi};
    wire mosi_data = q_mosi[1];

    // Receive bits
    reg [2:0] q_bitcnt;
    reg       q_byte_received;
    reg [7:0] q_rx_shift;

    always @(posedge clk)
        if (~ssel_active)
            q_bitcnt   <= 3'd0;
        else if (sclk_rising) begin
            q_bitcnt   <= q_bitcnt + 3'd1;
            q_rx_shift <= {q_rx_shift[6:0], mosi_data};
        end

    // Transmit bits
    reg [7:0] q_tx_shift;

    always @(posedge clk) begin
        txdata_ack <= 1'b0;

        if (sclk_falling) begin
            q_tx_shift <= {q_tx_shift[6:0], 1'b0};

            if (q_bitcnt == 3'd0) begin
                q_tx_shift <= txdata;
                txdata_ack <= 1'b1;
            end
        end
    end

    assign esp_miso = !esp_ssel_n ? q_tx_shift[7] : 1'bZ;

    // Byte completion
    always @(posedge clk)
        q_byte_received <= ssel_active && sclk_rising && (q_bitcnt == 3'd7);

    assign msg_start    = ssel_msg_start;
    assign msg_end      = ssel_msg_end;
    assign rxdata       = q_rx_shift;
    assign rxdata_valid = q_byte_received;

endmodule
