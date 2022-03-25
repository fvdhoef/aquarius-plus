module top(
    input  wire        sysclk,      // 14.31818MHz
    input  wire        reset_n,

    output wire        phi,

    // Z80 bus interface
    input  wire [15:0] bus_a,
    inout  wire  [7:0] bus_d,
    input  wire        bus_rd_n,
    input  wire        bus_wr_n,
    input  wire        bus_mreq_n,
    input  wire        bus_iorq_n,
    output wire        bus_int_n,   // Open-drain output

    inout  wire  [7:0] bus_de,      // External data bus, possibly scrambled
    output wire        ram_cs_n,    // 512KB RAM
    output wire        rom_cs_n,    // 256KB Flash memory
    output reg   [5:0] bank_a,

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output wire        cassette_out,
    input  wire        cassette_in,
    output wire        printer_out,
    input  wire        printer_in,

    // VGA output
    output wire  [3:0] vga_r,
    output wire  [3:0] vga_g,
    output wire  [3:0] vga_b,
    output wire        vga_hsync,
    output wire        vga_vsync,

    // ESP32 serial interface
    output wire        esp_tx,
    input  wire        esp_rx,

    // ESP32 SPI interface (also used for loading FPGA image)
    input  wire        esp_cs_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso
);

    //////////////////////////////////////////////////////////////////////////
    // Generate phi signal
    //////////////////////////////////////////////////////////////////////////
    reg [1:0] phi_div_r = 2'b0;
    always @(posedge sysclk) phi_div_r <= phi_div_r + 2'b1;
    assign phi = phi_div_r[1];

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////
    wire iord  =  bus_rd_n && bus_iorq_n;
    wire iowr  = !bus_rd_n && bus_iorq_n;
    wire memrd =  bus_rd_n && bus_mreq_n;
    wire memwr = !bus_rd_n && bus_mreq_n;

    //////////////////////////////////////////////////////////////////////////
    // Banking registers
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] bank0_reg_r;
    reg [7:0] bank1_reg_r;
    reg [7:0] bank2_reg_r;
    reg [7:0] bank3_reg_r;  

    always @* case (bus_a[15:14])
        2'd0: bank_a = bank0_reg_r[5:0];
        2'd1: bank_a = bank1_reg_r[5:0];
        2'd2: bank_a = bank2_reg_r[5:0];
        2'd3: bank_a = bank3_reg_r[5:0];
    endcase

endmodule
