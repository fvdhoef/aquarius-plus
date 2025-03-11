`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_spi(
    input  wire        clk,
    input  wire        reset,

    // System information
    input  wire  [7:0] sysinfo_core_type,
    input  wire  [7:0] sysinfo_flags,
    input  wire  [7:0] sysinfo_version_major,
    input  wire  [7:0] sysinfo_version_minor,

    input  wire [127:0] core_name,

    // Interface for core specific messages
    output wire        spi_msg_end,
    output wire  [7:0] spi_cmd,
    output wire [63:0] spi_rxdata,
    input  wire [63:0] spi_txdata,
    input  wire        spi_txdata_valid,

    // Display overlay interface
    output wire  [9:0] ovl_text_addr,
    output wire [15:0] ovl_text_wrdata,
    output wire        ovl_text_wr,

    output wire [10:0] ovl_font_addr,
    output wire  [7:0] ovl_font_wrdata,
    output wire        ovl_font_wr,

    output wire  [3:0] ovl_palette_addr,
    output wire [15:0] ovl_palette_wrdata,
    output wire        ovl_palette_wr,

    // ESP SPI slave interface
    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,
    output reg         esp_notify
);

    //////////////////////////////////////////////////////////////////////////
    // SPI slave
    //////////////////////////////////////////////////////////////////////////
    wire        msg_start, msg_end, rxdata_valid;
    wire  [7:0] rxdata;
    reg  [63:0] q_txdata = 0;
    wire        txdata_ack;

    aqp_spislave spislave(
        .clk(clk),

        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),

        .msg_start(msg_start),
        .msg_end(msg_end),
        .rxdata(rxdata),
        .rxdata_valid(rxdata_valid),
        .txdata(q_txdata[7:0]),
        .txdata_ack(txdata_ack));

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
    // reg        q_cmd_updated;

    always @(posedge clk) begin
        q_data_updated <= 1'b0;
        // q_cmd_updated  <= 1'b0;

        if (q_data_updated)
            q_byte_cnt <= q_byte_cnt + 11'd1;

        if (msg_start)
            q_state <= StCmd;
        if (msg_end)
            q_state <= StIdle;
        if (msg_start || msg_end)
            q_byte_cnt <= 11'd0;
        if (q_state == StCmd && rxdata_valid) begin
            q_cmd   <= rxdata;
            // q_cmd_updated <= 1'b1;
            q_state <= StData;
        end
        if (q_state == StData && rxdata_valid) begin
            q_data         <= {rxdata, q_data[63:8]};
            q_data_updated <= 1'b1;
        end
    end

    assign spi_msg_end = msg_end;
    assign spi_cmd     = q_cmd;
    assign spi_rxdata  = q_data;

    //////////////////////////////////////////////////////////////////////////
    // Commands
    //////////////////////////////////////////////////////////////////////////
    reg [1:0] q_tx_state;
    reg [7:0] q_status;

    localparam [7:0]
        CMD_OVL_TEXT    = 8'hF4,
        CMD_OVL_FONT    = 8'hF5,
        CMD_OVL_PALETTE = 8'hF6,
        CMD_GET_STATUS  = 8'hF7,
        CMD_GET_SYSINFO = 8'hF8,
        CMD_GET_NAME1   = 8'hF9,
        CMD_GET_NAME2   = 8'hFA;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_status   <= 8'h00;
            q_txdata   <= 64'b0;
            q_tx_state <= 2'd0;

        end else begin
            if (msg_start) begin
                q_txdata   <= 64'b0;
                q_tx_state <= 2'd0;
            end

            if (msg_end) begin
                case (q_cmd)
                    default: begin end
                endcase
            end

            if (q_state == StData && q_tx_state == 2'd0) begin
                case (q_cmd)
                    CMD_GET_STATUS: begin
                        q_txdata   <= {56'b0, q_status};
                        q_tx_state <= 2'd1;
                        q_status   <= 8'h00;
                    end

                    CMD_GET_SYSINFO: begin
                        q_txdata <= {
                            32'b0,
                            sysinfo_version_minor,
                            sysinfo_version_major,
                            sysinfo_flags,
                            sysinfo_core_type
                        };
                        q_tx_state <= 2'd1;
                    end

                    CMD_GET_NAME1: begin
                        q_txdata <= {
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
                        q_txdata <= {
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
                            q_txdata <= spi_txdata;
                            q_tx_state <= 2'd1;
                        end
                    end
                endcase
            end

            if (txdata_ack) begin
                if (q_tx_state == 2'd1)
                    q_tx_state <= 2'd2;

                if (q_tx_state == 2'd2)
                    q_txdata <= {8'h00, q_txdata[63:8]};
            end
        end
    end

    always @(posedge clk) esp_notify <= (q_status != 8'h0);

    // Overlay interface
    assign ovl_text_addr      = q_byte_cnt[10:1];
    assign ovl_text_wrdata    = q_data[63:48];
    assign ovl_text_wr        = (q_cmd == CMD_OVL_TEXT && q_byte_cnt[0] && q_data_updated);

    assign ovl_font_addr      = q_byte_cnt[10:0];
    assign ovl_font_wrdata    = q_data[63:56];
    assign ovl_font_wr        = (q_cmd == CMD_OVL_FONT && q_data_updated);

    assign ovl_palette_addr   = q_byte_cnt[4:1];
    assign ovl_palette_wrdata = q_data[63:48];
    assign ovl_palette_wr     = (q_cmd == CMD_OVL_PALETTE && q_byte_cnt[0] && q_data_updated);

endmodule
