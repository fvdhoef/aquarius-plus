`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_uart(
    input  wire        clk,
    input  wire        reset,

    input  wire  [8:0] txfifo_data,   // if bit8 set: transmit start-of-frame, ignore data
    input  wire        txfifo_wr,
    output wire        txfifo_full,

    output wire  [8:0] rxfifo_data,   // if bit8 set: received start-of-frame, other data bits will be 0
    input  wire        rxfifo_rd,
    output wire        rxfifo_empty,
    output wire        rxfifo_overflow,
    output wire        rx_framing_error,

    // ESP UART interface
    output wire        esp_tx,
    input  wire        esp_rx,
    output wire        esp_rts,
    input  wire        esp_cts
);

    // Synchronize CTS
    reg [1:0] q_cts;
    always @(posedge clk) q_cts <= {q_cts[0], esp_cts};

    //////////////////////////////////////////////////////////////////////////
    // UART TX
    //
    // Escaping:
    //   0x1xx will transmit 0x7E
    //   0x07D will transmit 0x7D 0x5D (0x7D ^ 0x20)
    //   0x07E will transmit 0x7D 0x5E (0x7E ^ 0x20)
    //   Other values are sent as is
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] txfifo_q;
    wire       tx_busy;
    wire       txfifo_empty;

    reg        q_tx_start;
    reg  [7:0] q_tx_data;
    reg  [1:0] q_tx_state;
    wire       tx_valid = (q_tx_state == 2'b00) && !q_tx_start && !q_cts[1] && !txfifo_empty && !tx_busy;

    wire       txfifo_almost_full; // unused

    aqp_esp_uart_fifo tx_fifo(
	    .clk(clk),
        .reset(reset),

    	.wrdata(txfifo_data),
	    .wr_en(txfifo_wr),

        .rddata(txfifo_q),
    	.rd_en(tx_valid),

    	.empty(txfifo_empty),
    	.full(txfifo_full),
        .almost_full(txfifo_almost_full));

    // State machine to send escaped data
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_tx_state <= 2'b00;
            q_tx_start <= 1'b0;
            q_tx_data  <= 8'h00;

        end else begin
            q_tx_start <= 1'b0;

            case (q_tx_state)
                2'b00: begin
                    if (tx_valid) begin
                        q_tx_state <= 2'b01;
                    end
                end

                2'b01: begin
                    q_tx_start <= 1'b1;
                    if (txfifo_q[8]) begin
                        // Send 'start of frame'
                        q_tx_data  <= 8'h7E;
                        q_tx_state <= 2'b00;
                    end else if (txfifo_q[7:0] == 8'h7D || txfifo_q[7:0] == 8'h7E) begin
                        // These bytes needs escaping, first send escape byte
                        q_tx_data  <= 8'h7D;
                        q_tx_state <= 2'b10;
                    end else begin
                        // Send as-is
                        q_tx_data  <= txfifo_q[7:0];
                        q_tx_state <= 2'b00;
                    end
                end

                2'b10: begin
                    if (!q_tx_start && !tx_busy) begin
                        // Now escape byte has been sent, send data with bit 5 inverted
                        q_tx_start <= 1'b1;
                        q_tx_data  <= txfifo_q[7:0] ^ 8'h20;
                        q_tx_state <= 2'b00;
                    end
                end

                default: q_tx_state <= 2'b00;
            endcase
        end
    end

    aqp_esp_uart_tx esp_uart_tx(
        .clk(clk),
        .reset(reset),
        .uart_txd(esp_tx),
        .tx_data(q_tx_data),
        .tx_valid(q_tx_start),
        .tx_busy(tx_busy));

    //////////////////////////////////////////////////////////////////////////
    // UART RX
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] rx_data;
    wire       rx_valid;
    wire       rxfifo_full;
    wire       rxfifo_almost_full;

    assign esp_rts = rxfifo_almost_full;
    wire overflow = rx_valid & rxfifo_full;
    wire framing_error;

    assign rxfifo_overflow = overflow;
    assign rx_framing_error = framing_error;

    aqp_esp_uart_rx esp_uart_rx(
        .clk(clk),
        .reset(reset),
        .uart_rxd(esp_rx),
        .rx_data(rx_data),
        .rx_valid(rx_valid),
        .framing_error(framing_error));

    reg [8:0] q_rxfifo_wrdata;
    reg       q_rxfifo_wr;
    reg       q_rx_escape;

    // State machine to receive escaped data
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_rx_escape     <= 1'b0;
            q_rxfifo_wrdata <= 9'h000;
            q_rxfifo_wr     <= 1'b0;

        end else begin
            q_rxfifo_wr <= 1'b0;

            if (rx_valid) begin
                if (rx_data == 8'h7E) begin // Start-of-frame?
                    q_rxfifo_wrdata <= 9'h100;
                    q_rxfifo_wr     <= 1'b1;
                    q_rx_escape     <= 1'b0;
                end else if (rx_data == 8'h7D) begin // Escape byte?
                    q_rx_escape     <= 1'b1;
                end else begin
                    q_rxfifo_wrdata <= {1'b0, rx_data ^ (q_rx_escape ? 8'h20 : 8'h00)};
                    q_rxfifo_wr     <= 1'b1;
                    q_rx_escape     <= 1'b0;
                end
            end
        end
    end

    aqp_esp_uart_fifo rx_fifo(
	    .clk(clk),
        .reset(reset),

    	.wrdata(q_rxfifo_wrdata),
	    .wr_en(q_rxfifo_wr),

        .rddata(rxfifo_data),
    	.rd_en(rxfifo_rd),

    	.empty(rxfifo_empty),
    	.full(rxfifo_full),
        .almost_full(rxfifo_almost_full));

endmodule
