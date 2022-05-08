module spislave(
    input  wire        clk,
    input  wire        reset,

    input  wire        esp_cs_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso);

    assign esp_miso = 1'bZ;

endmodule
