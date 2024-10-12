`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_spi(
    input  wire        clk,
    input  wire        reset,

    // Core specific
    output reg         reset_req,
    output reg  [63:0] keys,
    output reg   [7:0] hctrl1,
    output reg   [7:0] hctrl2,

    // Bus master interface
    input  wire        ebus_phi,

    output reg  [15:0] spibm_a,
    input  wire  [7:0] spibm_rddata,
    output reg   [7:0] spibm_wrdata,
    output reg         spibm_wrdata_en,
    output reg         spibm_rd_n,
    output reg         spibm_wr_n,
    output reg         spibm_mreq_n,
    output reg         spibm_iorq_n,
    output wire        spibm_busreq_n,
    
    // Interface for core specific messages
    output wire        spi_msg_end,
    output wire  [7:0] spi_cmd,
    output wire [63:0] spi_rxdata,

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
    output wire        esp_notify
);

    assign esp_notify = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // SPI slave
    //////////////////////////////////////////////////////////////////////////
    wire        msg_start, msg_end, rxdata_valid;
    wire  [7:0] rxdata;
    reg   [7:0] q_txdata = 0;
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
        .txdata(q_txdata),
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
    reg q_phi;
    always @(posedge clk) q_phi <= ebus_phi;
    wire phi_falling =  q_phi && !ebus_phi;
    wire phi_rising  = !q_phi &&  ebus_phi;

    reg q_busreq = 1'b0;
    assign spibm_busreq_n = !q_busreq;

    localparam [7:0]
        CMD_RESET           = 8'h01,
        CMD_SET_KEYB_MATRIX = 8'h10,
        CMD_SET_HCTRL       = 8'h11,
        CMD_BUS_ACQUIRE     = 8'h20,
        CMD_BUS_RELEASE     = 8'h21,
        CMD_MEM_WRITE       = 8'h22,
        CMD_MEM_READ        = 8'h23,
        CMD_IO_WRITE        = 8'h24,
        CMD_IO_READ         = 8'h25,
        CMD_OVL_TEXT        = 8'hF4,
        CMD_OVL_FONT        = 8'hF5,
        CMD_OVL_PALETTE     = 8'hF6;
        // CMD_GET_STATUS        = 8'hF7;

    // 01h: Reset command
    always @(posedge clk) begin
        reset_req <= 1'b0;
        if (q_cmd == CMD_RESET && msg_end) reset_req <= 1'b1;
    end

    // 10h: Set keyboard matrix
    always @(posedge clk or posedge reset)
        if (reset)
            keys <= 64'hFFFFFFFFFFFFFFFF;
        else if (q_cmd == CMD_SET_KEYB_MATRIX && msg_end)
            keys <= q_data;

    // 11h: Set hand controller
    always @(posedge clk or posedge reset)
        if (reset)
            {hctrl2, hctrl1} <= 16'hFFFF;
        else if (q_cmd == CMD_SET_HCTRL && msg_end)
            {hctrl2, hctrl1} <= q_data[63:48];

    // 20h/21h: Acquire/release bus
    always @(posedge clk) begin
        if (phi_falling) begin
            if (q_cmd == CMD_BUS_ACQUIRE) q_busreq <= 1'b1;
            if (q_cmd == CMD_BUS_RELEASE) q_busreq <= 1'b0;
        end
    end

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

    localparam
        BStIdle   = 3'd0,
        BStCycle0 = 3'd1,
        BStCycle1 = 3'd2,
        BStCycle2 = 3'd3,
        BStDone   = 3'd4;

    reg [2:0] q_bus_state = BStIdle;

    always @(posedge clk) begin
        case (q_bus_state)
            BStIdle: begin
                spibm_rd_n      <= 1'b1;
                spibm_wr_n      <= 1'b1;
                spibm_mreq_n    <= 1'b1;
                spibm_iorq_n    <= 1'b1;
                spibm_wrdata_en <= 1'b0;

                if (q_data_updated) begin
                    case (q_byte_cnt)
                        11'd0: spibm_a[7:0] <= q_data[63:56];
                        11'd1: begin
                            spibm_a[15:8] <= q_data[63:56];
                            if (q_cmd == CMD_MEM_READ || q_cmd == CMD_IO_READ) q_bus_state <= BStCycle0;
                        end
                        11'd2: begin
                            spibm_wrdata <= q_data[63:56];
                            if (q_cmd == CMD_MEM_WRITE || q_cmd == CMD_IO_WRITE) q_bus_state <= BStCycle0;
                        end

                        default: begin end
                    endcase
                end
            end

            BStCycle0: begin
                if (phi_falling) begin
                    spibm_mreq_n    <= !(q_cmd == CMD_MEM_READ  || q_cmd == CMD_MEM_WRITE);
                    spibm_iorq_n    <= !(q_cmd == CMD_IO_READ   || q_cmd == CMD_IO_WRITE);
                    spibm_rd_n      <= !(q_cmd == CMD_MEM_READ  || q_cmd == CMD_IO_READ);
                    spibm_wrdata_en <=  (q_cmd == CMD_MEM_WRITE || q_cmd == CMD_IO_WRITE);
                    
                    q_bus_state <= BStCycle1;
                end
            end

            BStCycle1: begin
                if (phi_falling) begin
                    spibm_wr_n <= !(q_cmd == CMD_MEM_WRITE || q_cmd == CMD_IO_WRITE);

                    q_bus_state <= BStCycle2;
                end
            end

            BStCycle2: begin
                if (phi_falling) begin
                    if (q_cmd == CMD_MEM_READ || q_cmd == CMD_IO_READ)
                        q_txdata <= spibm_rddata;

                    spibm_mreq_n <= 1'b1;
                    spibm_iorq_n <= 1'b1;
                    spibm_rd_n   <= 1'b1;
                    spibm_wr_n   <= 1'b1;

                    q_bus_state <= BStDone;
                end
            end

            BStDone: begin
                if (phi_rising) begin
                    spibm_wrdata_en <= 1'b0;
                end
            end

            default: begin end

        endcase

        if (msg_start) q_bus_state <= BStIdle;
    end

endmodule
