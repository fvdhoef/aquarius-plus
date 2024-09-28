`default_nettype none
`timescale 1 ns / 1 ps

module aqp_esp_uart(
    input  wire        clk,
    input  wire        reset,

    input  wire  [7:0] tx_data,
    input  wire        tx_valid,
    input  wire        tx_break,
    output wire        tx_busy,

    output reg   [7:0] rxfifo_data,
    output wire        rxfifo_not_empty,
    input  wire        rxfifo_read,
    output wire        rxfifo_overflow,
    output wire        rx_framing_error,
    output wire        rx_break,

    input  wire        uart_rxd,
    output wire        uart_txd,
    input  wire        uart_cts,
    output wire        uart_rts);

    //////////////////////////////////////////////////////////////////////////
    // UART TX
    //////////////////////////////////////////////////////////////////////////
    aqp_esp_uart_tx esp_uart_tx(
        .clk(clk),
        .reset(reset),
        .uart_txd(uart_txd),
        .tx_data(tx_data),
        .tx_valid(tx_valid),
        .tx_break(tx_break),
        .tx_busy(tx_busy));

    //////////////////////////////////////////////////////////////////////////
    // UART RX
    //////////////////////////////////////////////////////////////////////////
    wire [7:0] rx_data;
    wire       rx_valid;
    wire [7:0] rxfifo_rddata;
    wire       rxfifo_empty;
    wire       rxfifo_full;
    wire       rxfifo_almost_full;

    always @(posedge clk) if (rxfifo_read) rxfifo_data <= rxfifo_rddata;

    aqp_esp_uart_rxfifo esp_uart_rxfifo(
	    .clk(clk),
        .reset(reset),

    	.wrdata(rx_data),
	    .wr_en(rx_valid),

        .rddata(rxfifo_rddata),
    	.rd_en(rxfifo_read),

    	.empty(rxfifo_empty),
    	.full(rxfifo_full),
        .almost_full(rxfifo_almost_full));

    assign rxfifo_not_empty = !rxfifo_empty;
    assign rxfifo_overflow  = rx_valid && rxfifo_full;

    assign uart_rts = rxfifo_almost_full;

    //////////////////////////////////////////////////////////////////////////
    // UART RX - 'uart_clk' clock domain
    //////////////////////////////////////////////////////////////////////////
    wire rx_fe, rx_brk;
    reg q_rx_fe, q_rx_brk;

    always @(posedge clk) q_rx_fe  <= rx_fe;
    always @(posedge clk) q_rx_brk <= rx_brk;

    assign rx_framing_error = rx_fe  && !q_rx_fe;
    assign rx_break         = rx_brk && !q_rx_brk;

    aqp_esp_uart_rx esp_uart_rx(
        .clk(clk),
        .reset(reset),
        .uart_rxd(uart_rxd),
        .rx_data(rx_data),
        .rx_valid(rx_valid),
        
        .framing_error(rx_fe),
        .brk(rx_brk));

endmodule
