module spiregs(
    input  wire        clk,
    input  wire        reset,

    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,

    input  wire        ebus_phi,

    output reg  [15:0] spibm_a,
    input  wire  [7:0] spibm_rddata,
    output reg   [7:0] spibm_wrdata,
    output reg         spibm_wrdata_en,
    output reg         spibm_rd_n,
    output reg         spibm_wr_n,
    output reg         spibm_mreq_n,
    output reg         spibm_iorq_n,
    output wire        spibm_busreq,

    output reg         reset_req,
    output reg  [63:0] keys,
    output reg   [7:0] hctrl1,
    output reg   [7:0] hctrl2,
    output reg         rom_p2_wren,
    
    output reg   [7:0] kbbuf_data,
    output reg         kbbuf_wren,
    
    output wire        video_mode);

    //////////////////////////////////////////////////////////////////////////
    // SPI slave
    //////////////////////////////////////////////////////////////////////////
    wire        msg_start, msg_end, rxdata_valid;
    wire  [7:0] rxdata;

    reg   [7:0] txdata_r = 8'h00;
    wire        txdata_ack;

    spislave spislave(
        .clk(clk),

        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),
        
        .msg_start(msg_start),
        .msg_end(msg_end),
        .rxdata(rxdata),
        .rxdata_valid(rxdata_valid),
        .txdata(txdata_r),
        .txdata_ack(txdata_ack));

    //////////////////////////////////////////////////////////////////////////
    // Data reception
    //////////////////////////////////////////////////////////////////////////
    reg [63:0] data_r;

    localparam
        ST_IDLE = 2'b00,
        ST_CMD  = 2'b01,
        ST_DATA = 2'b10;

    reg [1:0] state_r;
    reg [7:0] cmd_r;
    reg [2:0] byte_cnt_r;

    always @(posedge clk) begin
        if (msg_start)
            state_r <= ST_CMD;
        if (msg_end)
            state_r <= ST_IDLE;
        if (msg_start || msg_end)
            byte_cnt_r <= 3'b0;
        if (rxdata_valid)
            byte_cnt_r <= byte_cnt_r + 3'b1;

        if (state_r == ST_CMD && rxdata_valid) begin
            cmd_r   <= rxdata;
            state_r <= ST_DATA;
        end

        if (state_r == ST_DATA && rxdata_valid) begin
            data_r <= {rxdata, data_r[63:8]};
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Commands
    //////////////////////////////////////////////////////////////////////////
    reg phi_r;
    always @(posedge clk) phi_r <= ebus_phi;
    wire phi_falling = phi_r && !ebus_phi;
    wire phi_rising = !phi_r && ebus_phi;

    reg busreq_r = 1'b0;
    assign spibm_busreq = busreq_r;

    localparam
        CMD_RESET           = 8'h01,
        CMD_SET_KEYB_MATRIX = 8'h10,
        CMD_SET_HCTRL       = 8'h11,
        CMD_WRITE_KBBUF     = 8'h12,
        CMD_BUS_ACQUIRE     = 8'h20,
        CMD_BUS_RELEASE     = 8'h21,
        CMD_MEM_WRITE       = 8'h22,
        CMD_MEM_READ        = 8'h23,
        CMD_IO_WRITE        = 8'h24,
        CMD_IO_READ         = 8'h25,
        CMD_ROM_WRITE       = 8'h30,
        CMD_SET_VIDMODE     = 8'h40;

    // 01h: Reset command
    always @(posedge clk) begin
        reset_req <= 1'b0;
        if (cmd_r == CMD_RESET && msg_end) reset_req <= 1'b1;
    end

    // 10h: Set keyboard matrix
    always @(posedge clk)
        if (reset)
            keys <= 64'hFFFFFFFFFFFFFFFF;
        else if (cmd_r == CMD_SET_KEYB_MATRIX && msg_end)
            keys <= data_r;

    // 11h: Set hand controller
    always @(posedge clk)
        if (reset)
            {hctrl2, hctrl1} <= 16'hFFFF;
        else if (cmd_r == CMD_SET_HCTRL && msg_end)
            {hctrl2, hctrl1} <= data_r[63:48];

    // 12h: Write keyboard buffer
    always @(posedge clk)
        if (reset) begin
            kbbuf_data <= 8'h00;
            kbbuf_wren <= 1'b0;
        end else begin
            kbbuf_wren <= 1'b0;
            if (cmd_r == CMD_WRITE_KBBUF && msg_end) begin
                kbbuf_data <= data_r[63:56];
                kbbuf_wren <= 1'b1;
            end
        end

    // 20h/21h: Acquire/release bus
    always @(posedge clk) begin
        if (phi_falling) begin
            if (cmd_r == CMD_BUS_ACQUIRE) busreq_r <= 1'b1;
            if (cmd_r == CMD_BUS_RELEASE) busreq_r <= 1'b0;
        end
    end

    // 40h: Set video mode
    reg video_mode_r = 1'b0;
    always @(posedge clk) begin
        if (cmd_r == CMD_SET_VIDMODE && msg_end) begin
            video_mode_r <= data_r[56];
        end
    end
    assign video_mode = video_mode_r;

    localparam
        BST_IDLE   = 3'd0,
        BST_CYCLE0 = 3'd1,
        BST_CYCLE1 = 3'd2,
        BST_CYCLE2 = 3'd3,
        BST_DONE   = 3'd4;

    reg [2:0] bus_state_r = BST_IDLE;

    always @(posedge clk) begin
        rom_p2_wren <= 1'b0;

        case (bus_state_r)
            BST_IDLE: begin
                spibm_rd_n      <= 1'b1;
                spibm_wr_n      <= 1'b1;
                spibm_mreq_n    <= 1'b1;
                spibm_iorq_n    <= 1'b1;
                spibm_wrdata_en <= 1'b0;

                // txdata_r <= 8'h00;

                if (rxdata_valid) begin
                    case (byte_cnt_r)
                        3'd1: spibm_a[7:0] <= rxdata;
                        3'd2: begin
                            spibm_a[15:8] <= rxdata;
                            if (cmd_r == CMD_MEM_READ || cmd_r == CMD_IO_READ) bus_state_r <= BST_CYCLE0;
                        end
                        3'd3: begin
                            spibm_wrdata <= rxdata;
                            if (cmd_r == CMD_MEM_WRITE || cmd_r == CMD_IO_WRITE) bus_state_r <= BST_CYCLE0;
                            if (cmd_r == CMD_ROM_WRITE) rom_p2_wren <= 1'b1;
                        end
                    endcase
                end
            end

            BST_CYCLE0: begin
                if (phi_falling) begin
                    spibm_mreq_n    <= !(cmd_r == CMD_MEM_READ  || cmd_r == CMD_MEM_WRITE);
                    spibm_iorq_n    <= !(cmd_r == CMD_IO_READ   || cmd_r == CMD_IO_WRITE);
                    spibm_rd_n      <= !(cmd_r == CMD_MEM_READ  || cmd_r == CMD_IO_READ);
                    spibm_wrdata_en <=  (cmd_r == CMD_MEM_WRITE || cmd_r == CMD_IO_WRITE);
                    
                    bus_state_r <= BST_CYCLE1;
                end
            end

            BST_CYCLE1: begin
                if (phi_falling) begin
                    spibm_wr_n <= !(cmd_r == CMD_MEM_WRITE || cmd_r == CMD_IO_WRITE);

                    bus_state_r <= BST_CYCLE2;
                end
            end

            BST_CYCLE2: begin
                if (phi_falling) begin
                    if (cmd_r == CMD_MEM_READ || cmd_r == CMD_IO_READ)
                        txdata_r <= spibm_rddata;

                    spibm_mreq_n <= 1'b1;
                    spibm_iorq_n <= 1'b1;
                    spibm_rd_n   <= 1'b1;
                    spibm_wr_n   <= 1'b1;

                    bus_state_r <= BST_DONE;
                end
            end

            BST_DONE: begin
                if (phi_rising) begin
                    spibm_wrdata_en <= 1'b0;
                end
            end

        endcase

        if (msg_start) bus_state_r <= BST_IDLE;
    end

endmodule
