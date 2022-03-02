// Verilog description for Mattel Aquarius PLA2
//
// This PLA is responsible for:
// - Latching the scramble register value (write to IO address $FF)
// - Scrambling/descrambling external bus
// - Connecting the internal bus to either the external data bus or color RAM data bus
// - Latching data from color RAM and multiplexing this data to the RGBI output
//   pins depending on the DOT input, which comes from the video shift register.
//
module pla2(
    input  wire       dot,      // pin 1 - Current pixel value
    input  wire       phi4_n,   // pin 3 - Character clock
    input  wire       laeb_n,   // pin 4 - 
    input  wire       exbe_n,   // pin 5 - External bus enable?
    input  wire       dbe_n,    // pin 6 - Color RAM data bus enable?
                                // pin 7/8 not connected
    inout  wire [7:0] d,        // pin 9-16
    input  wire       iorq_n,   // pin 17 - IO request
    input  wire       wr_n,     // pin 18 - Write
    input  wire       mrst_n,   // pin 19 - Reset
    output wire       r,        // pin 20 - Red output
    output wire       g,        // pin 21 - Green output
    output wire       b,        // pin 22 - Blue output
    output wire       inv,      // pin 23 - Invert output
    inout  wire [7:0] de,       // pin 24-31 - External data bus
    inout  wire [7:0] cd        // pin 32-40 - Color RAM data bus
);

    // Tristate buffer for Color RAM data bus
    wire cd_oe = (wr_n == 1'b0);    // FIXME!
    assign cd = cd_oe ? d : 8'bZ;

    // Tristate buffer for external data bus
    wire de_oe = (wr_n == 1'b0);     // FIXME!
    assign de = d_oe ? d : 8'bZ;

    // Tristate buffer for internal data bus
    wire d_oe = 1'b0;
    assign d = d_oe ? d : 8'bZ;

    // Color data latch
    reg [7:0] color_data;
    always @ (phi4_n or cd)
        if (!phi4_n) color_data <= cd;

    // Color output multiplexer
    assign r   = dot ? color_data[0] : color_data[4];
    assign g   = dot ? color_data[1] : color_data[5];
    assign b   = dot ? color_data[2] : color_data[6];
    assign inv = dot ? color_data[3] : color_data[7];

endmodule
