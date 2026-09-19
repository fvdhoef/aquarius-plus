`default_nettype none
`timescale 1 ns / 1 ps

module fpga_spislave(
    input  wire         clk,
    input  wire         reset,

    // System information
    input  wire   [7:0] core_type,
    input  wire   [7:0] core_flags,
    input  wire  [15:0] core_version,
    input  wire [127:0] core_name,

    // Interface for core specific messages
    output wire         spi_msg_end,
    output wire   [7:0] spi_cmd,
    output wire  [63:0] spi_rxdata,
    input  wire  [63:0] spi_txdata,
    input  wire         spi_txdata_valid,

    // Display overlay interface
    output wire  [9:0] ovl_text_addr,
    output wire [15:0] ovl_text_wrdata,
    output wire        ovl_text_wren,

    output wire [10:0] ovl_font_addr,
    output wire  [7:0] ovl_font_wrdata,
    output wire        ovl_font_wren,

    output wire  [3:0] ovl_palette_addr,
    output wire [15:0] ovl_palette_wrdata,
    output wire        ovl_palette_wren,

    // Command signals
    output reg         reset_req,
    output reg   [7:0] hctrl1,
    output reg   [7:0] hctrl2,
    output reg  [63:0] keys,
    output reg  [63:0] gamepad1,
    output reg  [63:0] gamepad2,
    output reg  [15:0] kbbuf16_wrdata,
    output reg         kbbuf16_wren,

    // SPI interface
    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    reg [63:0] q_tx_data     = 0;
    reg        q_tx_data_ack = 0;

    //////////////////////////////////////////////////////////////////////////
    // Low-level SPI interface
    //////////////////////////////////////////////////////////////////////////

    // Synchronize SCLK
    reg [2:0] q_sclk;
    always @(posedge clk) q_sclk <= {q_sclk[1:0], esp_sclk};
    wire sclk_rising  = (q_sclk[2:1] == 2'b01);
    wire sclk_falling = (q_sclk[2:1] == 2'b10);

    // Synchronize SSEL#
    reg [2:0] q_ssel_n;
    always @(posedge clk) q_ssel_n <= {q_ssel_n[1:0], esp_ssel_n};
    wire ssel_active = ~q_ssel_n[1];
    wire msg_start   = (q_ssel_n[2:1] == 2'b10);
    wire msg_end     = (q_ssel_n[2:1] == 2'b01);

    // Synchronize MOSI
    reg [1:0] q_mosi;
    always @(posedge clk) q_mosi <= {q_mosi[0], esp_mosi};
    wire mosi_data = q_mosi[1];

    // Receive bits
    reg [2:0] q_bitcnt;
    reg       q_byte_received;
    reg [7:0] q_rx_data;

    always @(posedge clk)
        if (~ssel_active)
            q_bitcnt  <= 0;
        else if (sclk_rising) begin
            q_bitcnt  <= q_bitcnt + 3'd1;
            q_rx_data <= {q_rx_data[6:0], mosi_data};
        end

    // Transmit bits
    reg [7:0] q_tx_shift;

    always @(posedge clk) begin
        q_tx_data_ack <= 0;

        if (sclk_falling) begin
            q_tx_shift <= {q_tx_shift[6:0], 1'b0};

            if (q_bitcnt == 3'd0) begin
                q_tx_shift    <= q_tx_data[7:0];
                q_tx_data_ack <= 1;
            end
        end
    end

    assign esp_miso = !esp_ssel_n ? q_tx_shift[7] : 1'bZ;

    // Byte completion
    always @(posedge clk)
        q_byte_received <= ssel_active && sclk_rising && (q_bitcnt == 3'd7);

    //////////////////////////////////////////////////////////////////////////
    // Data reception
    //////////////////////////////////////////////////////////////////////////
    reg [63:0] q_data;

    localparam [1:0]
        StIdle = 2'b00,
        StCmd  = 2'b01,
        StData = 2'b10;

    reg  [1:0] q_state = StIdle;
    reg  [7:0] q_cmd;
    reg [10:0] q_byte_cnt;
    reg        q_data_updated;

    always @(posedge clk) begin
        q_data_updated <= 0;

        if (q_data_updated)       q_byte_cnt <= q_byte_cnt + 11'd1;
        if (msg_start)            q_state    <= StCmd;
        if (msg_end)              q_state    <= StIdle;
        if (msg_start || msg_end) q_byte_cnt <= 0;

        if (q_state == StCmd && q_byte_received) begin
            q_cmd   <= q_rx_data;
            q_state <= StData;
        end

        if (q_state == StData && q_byte_received) begin
            q_data         <= {q_rx_data, q_data[63:8]};
            q_data_updated <= 1;
        end
    end

    assign spi_msg_end = msg_end;
    assign spi_cmd     = q_cmd;
    assign spi_rxdata  = q_data;

    //////////////////////////////////////////////////////////////////////////
    // Command handling
    //////////////////////////////////////////////////////////////////////////
    reg [1:0] q_tx_state = 0;
    reg [7:0] q_status   = 0;

    localparam [7:0]
        CMD_RESET           = 8'h01,
        CMD_SET_HCTRL       = 8'h11,
        CMD_SET_KEYB_MATRIX = 8'h10,
        CMD_WRITE_KBBUF16   = 8'h13,
        CMD_WRITE_GAMEPAD1  = 8'h14,
        CMD_WRITE_GAMEPAD2  = 8'h15,
        CMD_OVL_TEXT        = 8'hF4,
        CMD_OVL_FONT        = 8'hF5,
        CMD_OVL_PALETTE     = 8'hF6,
        CMD_GET_SYSINFO     = 8'hF8,
        CMD_GET_NAME1       = 8'hF9,
        CMD_GET_NAME2       = 8'hFA;

    always @(posedge clk) begin
        if (msg_start) begin
            q_tx_data  <= 0;
            q_tx_state <= 0;
        end

        if (q_state == StData && q_tx_state == 2'd0) begin
            case (q_cmd)
                CMD_GET_SYSINFO: begin
                    q_tx_data <= {
                        32'b0,
                        core_version[7:0],
                        core_version[15:8],
                        core_flags,
                        core_type
                    };
                    q_tx_state <= 2'd1;
                end

                CMD_GET_NAME1: begin
                    q_tx_data <= {
                        core_name[71:64],
                        core_name[79:72],
                        core_name[87:80],
                        core_name[95:88],
                        core_name[103:96],
                        core_name[111:104],
                        core_name[119:112],
                        core_name[127:120]
                    };
                    q_tx_state <= 2'd1; 
                end

                CMD_GET_NAME2: begin
                    q_tx_data <= {
                        core_name[7:0],
                        core_name[15:8],
                        core_name[23:16],
                        core_name[31:24],
                        core_name[39:32],
                        core_name[47:40],
                        core_name[55:48],
                        core_name[63:56]
                    };
                    q_tx_state <= 2'd1;
                end

                default: begin
                    if (spi_txdata_valid) begin
                        q_tx_data  <= spi_txdata;
                        q_tx_state <= 2'd1;
                    end
                end
            endcase
        end

        if (q_tx_data_ack) begin
            if (q_tx_state == 2'd1)
                q_tx_state <= 2'd2;

            if (q_tx_state == 2'd2)
                q_tx_data <= {8'h00, q_tx_data[63:8]};
        end
    end

    // 01h: Reset command
    always @(posedge clk) begin
        reset_req <= 0;
        if (spi_cmd == CMD_RESET && spi_msg_end) begin
            reset_req <= 1;
        end
    end

    // 10h: Set keyboard matrix
    always @(posedge clk or posedge reset)
        if (reset)
            keys <= 64'hFFFFFFFFFFFFFFFF;
        else if (spi_cmd == CMD_SET_KEYB_MATRIX && spi_msg_end)
            keys <= spi_rxdata;

    // 11h: Set hand controllers
    always @(posedge clk or posedge reset)
        if (reset)
            {hctrl2, hctrl1} <= 16'hFFFF;
        else if (spi_cmd == CMD_SET_HCTRL && spi_msg_end)
            {hctrl2, hctrl1} <= spi_rxdata[63:48];

    // 13h: Write keyboard buffer (16-bit)
    always @(posedge clk or posedge reset)
        if (reset) begin
            kbbuf16_wrdata <= 0;
            kbbuf16_wren   <= 0;
        end else begin
            kbbuf16_wren <= 0;
            if (spi_cmd == CMD_WRITE_KBBUF16 && spi_msg_end) begin
                kbbuf16_wrdata <= spi_rxdata[63:48];
                kbbuf16_wren   <= 1;
            end
        end

    // 14h: Set gamepad1
    always @(posedge clk or posedge reset)
        if (reset)
            gamepad1 <= 0;
        else if (spi_cmd == CMD_WRITE_GAMEPAD1 && spi_msg_end)
            gamepad1 <= spi_rxdata;

    // 15h: Set gamepad2
    always @(posedge clk or posedge reset)
        if (reset)
            gamepad2 <= 0;
        else if (spi_cmd == CMD_WRITE_GAMEPAD2 && spi_msg_end)
            gamepad2 <= spi_rxdata;

    assign esp_notify = 0;

    assign ovl_text_addr      = q_byte_cnt[10:1];
    assign ovl_text_wrdata    = q_data[63:48];
    assign ovl_text_wren      = (q_cmd == CMD_OVL_TEXT && q_byte_cnt[0] && q_data_updated);

    assign ovl_font_addr      = q_byte_cnt[10:0];
    assign ovl_font_wrdata    = q_data[63:56];
    assign ovl_font_wren      = (q_cmd == CMD_OVL_FONT && q_data_updated);

    assign ovl_palette_addr   = q_byte_cnt[4:1];
    assign ovl_palette_wrdata = q_data[63:48];
    assign ovl_palette_wren   = (q_cmd == CMD_OVL_PALETTE && q_byte_cnt[0] && q_data_updated);

endmodule
