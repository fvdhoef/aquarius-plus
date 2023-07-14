module spislave(
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
    reg [2:0] sclk_r;
    always @(posedge clk) sclk_r <= {sclk_r[1:0], esp_sclk};
    wire sclk_rising  = (sclk_r[2:1] == 2'b01);
    wire sclk_falling = (sclk_r[2:1] == 2'b10);

    // Synchronize SSEL#
    reg [2:0] ssel_r;
    always @(posedge clk) ssel_r <= {ssel_r[1:0], esp_ssel_n};
    wire ssel_active    = ~ssel_r[1];
    wire ssel_msg_start = (ssel_r[2:1] == 2'b10);
    wire ssel_msg_end   = (ssel_r[2:1] == 2'b01);

    // Synchronize MOSI
    reg [1:0] mosi_r;
    always @(posedge clk) mosi_r <= {mosi_r[0], esp_mosi};
    wire mosi_data = mosi_r[1];

    // Receive bits
    reg [2:0] bitcnt_r;
    reg       byte_received;
    reg [7:0] rx_shift_r;

    always @(posedge clk)
        if (~ssel_active)
            bitcnt_r <= 3'd0;
        else if (sclk_rising) begin
            bitcnt_r <= bitcnt_r + 3'd1;
            rx_shift_r <= {rx_shift_r[6:0], mosi_data};
        end

    // Transmit bits
    reg [7:0] tx_shift_r;

    always @(posedge clk) begin
        txdata_ack <= 1'b0;
        
        if (sclk_falling) begin
            tx_shift_r <= {tx_shift_r[6:0], 1'b0};

            if (bitcnt_r == 3'd0) begin
                tx_shift_r <= txdata;
                txdata_ack <= 1'b1;
            end
        end
    end

    assign esp_miso = !esp_ssel_n ? tx_shift_r[7] : 1'bZ;

    // Byte completion
    always @(posedge clk)
        byte_received <= ssel_active && sclk_rising && (bitcnt_r == 3'd7);

    assign msg_start    = ssel_msg_start;
    assign msg_end      = ssel_msg_end;
    assign rxdata       = rx_shift_r;
    assign rxdata_valid = byte_received;

endmodule
