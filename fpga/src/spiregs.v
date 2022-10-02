module spiregs(
    input  wire        clk,
    input  wire        reset,

    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,

    output reg  [63:0] keys);

    //////////////////////////////////////////////////////////////////////////
    // SPI slave
    //////////////////////////////////////////////////////////////////////////
    wire        msg_start, msg_end, rxdata_valid;
    wire  [7:0] rxdata;

    spislave spislave(
        .clk(clk),
        .reset(reset),

        .esp_ssel_n(esp_ssel_n),
        .esp_sclk(esp_sclk),
        .esp_mosi(esp_mosi),
        .esp_miso(esp_miso),
        
        .msg_start(msg_start),
        .msg_end(msg_end),
        .rxdata(rxdata),
        .rxdata_valid(rxdata_valid));

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

    always @(posedge clk) begin
        if (msg_start)
            state_r <= ST_CMD;
        if (msg_end)
            state_r <= ST_IDLE;

        if (state_r == ST_CMD && rxdata_valid) begin
            cmd_r   <= rxdata;
            state_r <= ST_DATA;
        end

        if (state_r == ST_DATA && rxdata_valid) begin
            data_r <= {rxdata, data_r[63:8]};
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Keyboard register
    //////////////////////////////////////////////////////////////////////////
    always @(posedge clk)
        if (cmd_r == 8'h10 && msg_end) keys <= data_r;

endmodule
