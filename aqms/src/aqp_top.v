`default_nettype none
`timescale 1 ns / 1 ps

module aqp_top(
    input  wire        sysclk,          // 14.31818MHz

    // Z80 bus interface
    inout  wire        ebus_reset_n,
    output wire        ebus_phi,        // 3.579545MHz
    inout  wire [15:0] ebus_a,
    inout  wire  [7:0] ebus_d,
    inout  wire        ebus_rd_n,
    inout  wire        ebus_wr_n,
    inout  wire        ebus_mreq_n,
    inout  wire        ebus_iorq_n,
    output wire        ebus_int_n,      // Open-drain output
    output wire        ebus_busreq_n,   // Open-drain output
    input  wire        ebus_busack_n,
    output wire  [4:0] ebus_ba,
    output wire        ebus_ram_ce_n,   // 512KB RAM
    output wire        ebus_cart_ce_n,  // Cartridge
    output wire        ebus_ram_we_n,

    // PWM audio outputs
    output wire        audio_l,
    output wire        audio_r,

    // Other
    output wire        cassette_out,
    input  wire        cassette_in,
    output wire        printer_out,
    input  wire        printer_in,

    // Misc
    output wire  [8:0] exp,
    input  wire        has_z80,

    // Hand controller interface
    inout  wire  [8:0] hc1,
    inout  wire  [8:0] hc2,

    // VGA output
    output wire  [3:0] vga_r,
    output wire  [3:0] vga_g,
    output wire  [3:0] vga_b,
    output wire        vga_hsync,
    output wire        vga_vsync,

    // ESP32 serial interface
    output wire        esp_tx,
    input  wire        esp_rx,
    output wire        esp_rts,
    input  wire        esp_cts,

    // ESP32 SPI interface (also used for loading FPGA image)
    input  wire        esp_ssel_n,
    input  wire        esp_sclk,
    input  wire        esp_mosi,
    output wire        esp_miso,
    output wire        esp_notify
);

    assign exp = 9'b0;

    wire [15:0] spibm_a;
    wire  [7:0] spibm_wrdata;
    wire        spibm_wrdata_en;
    wire        spibm_rd_n, spibm_wr_n, spibm_mreq_n, spibm_iorq_n;
    wire        spibm_busreq_n;

    wire        iorq;
    wire        mreq;
    wire        use_t80;

    //////////////////////////////////////////////////////////////////////////
    // Clock synthesizer
    //////////////////////////////////////////////////////////////////////////
    wire clk;
    wire video_clk;
    wire video_mode;

    aqp_clkctrl clkctrl(
        .clk_in     ( sysclk     ),     // 14.31818MHz
        .clk_out    ( clk        ),     // 28.63636MHz

        .video_clk  ( video_clk  ),
        .video_mode ( video_mode )
    );

    //////////////////////////////////////////////////////////////////////////
    // System controller (reset and clock generation)
    //////////////////////////////////////////////////////////////////////////
    wire reset_req;
    wire turbo;
    wire turbo_unlimited;
    wire ebus_phi_clken;
    wire reset;

    aqp_sysctrl sysctrl(
        .sysclk          ( clk                        ),
        .ebus_reset_n    ( ebus_reset_n               ),
        .reset_req       ( reset_req                  ),

        .turbo_mode      ( turbo                      ),
        .turbo_unlimited ( turbo_unlimited && use_t80 ),

        .ebus_phi        ( ebus_phi                   ),
        .ebus_phi_clken  ( ebus_phi_clken             ),
        .reset           ( reset                      )
    );

    //////////////////////////////////////////////////////////////////////////
    // ESP SPI slave interface
    //////////////////////////////////////////////////////////////////////////
    wire        spi_msg_end;
    wire  [7:0] spi_cmd;
    wire [63:0] spi_rxdata;
    wire [63:0] spi_txdata;
    wire        spi_txdata_valid;

    wire  [9:0] ovl_text_addr;
    wire [15:0] ovl_text_wrdata;
    wire        ovl_text_wr;

    wire [10:0] ovl_font_addr;
    wire  [7:0] ovl_font_wrdata;
    wire        ovl_font_wr;

    wire  [3:0] ovl_palette_addr;
    wire [15:0] ovl_palette_wrdata;
    wire        ovl_palette_wr;

    aqp_esp_spi esp_spi(
        .clk                   ( clk                ),
        .reset                 ( reset              ),

        // System information
        .sysinfo_core_type     ( 8'h01              ),
        .sysinfo_flags         ({
            1'b0,       // Core type 01 specific: unused
            1'b0,       // Core type 01 specific: unused
            1'b0,       // Core type 01 specific: alternate baud rate
            1'b0,       // Core type 01 specific: show force turbo mode
            1'b0,       // Core type 01 specific: show Aquarius+ options
            1'b0,       // Core type 01 specific: show video timing switch
            1'b0,       // Core type 01 specific: show mouse support
            has_z80     // Z80 present
        }),
        .sysinfo_version_major ( 8'h01              ),
        .sysinfo_version_minor ( 8'h00              ),

        .core_name             ( "Master System   " ),

        // Bus master interface
        .ebus_phi              ( ebus_phi           ),

        .spibm_a               ( spibm_a            ),
        .spibm_rddata          ( ebus_d             ),
        .spibm_wrdata          ( spibm_wrdata       ),
        .spibm_wrdata_en       ( spibm_wrdata_en    ),
        .spibm_rd_n            ( spibm_rd_n         ),
        .spibm_wr_n            ( spibm_wr_n         ),
        .spibm_mreq_n          ( spibm_mreq_n       ),
        .spibm_iorq_n          ( spibm_iorq_n       ),
        .spibm_busreq_n        ( spibm_busreq_n     ),

        // Interface for core specific messages
        .spi_msg_end           ( spi_msg_end        ),
        .spi_cmd               ( spi_cmd            ),
        .spi_rxdata            ( spi_rxdata         ),
        .spi_txdata            ( spi_txdata         ),
        .spi_txdata_valid      ( spi_txdata_valid   ),

        // Display overlay interface
        .ovl_text_addr         ( ovl_text_addr      ),
        .ovl_text_wrdata       ( ovl_text_wrdata    ),
        .ovl_text_wr           ( ovl_text_wr        ),

        .ovl_font_addr         ( ovl_font_addr      ),
        .ovl_font_wrdata       ( ovl_font_wrdata    ),
        .ovl_font_wr           ( ovl_font_wr        ),

        .ovl_palette_addr      ( ovl_palette_addr   ),
        .ovl_palette_wrdata    ( ovl_palette_wrdata ),
        .ovl_palette_wr        ( ovl_palette_wr     ),

        // ESP SPI slave interface
        .esp_ssel_n            ( esp_ssel_n         ),
        .esp_sclk              ( esp_sclk           ),
        .esp_mosi              ( esp_mosi           ),
        .esp_miso              ( esp_miso           ),
        .esp_notify            ( esp_notify         )
    );

    //////////////////////////////////////////////////////////////////////////
    // PWM DAC
    //////////////////////////////////////////////////////////////////////////
    reg [15:0] common_audio_l;
    reg [15:0] common_audio_r;

    aqp_pwm_dac pwm_dac(
        .clk         ( clk            ),

        // Sample input
        .left_data   ( common_audio_l ),
        .right_data  ( common_audio_r ),

        // PWM audio output
        .audio_l     ( audio_l        ),
        .audio_r     ( audio_r        )
    );

    //////////////////////////////////////////////////////////////////////////
    // Core common
    //////////////////////////////////////////////////////////////////////////
    wire [3:0] video_r;
    wire [3:0] video_g;
    wire [3:0] video_b;
    wire       video_de;
    wire       video_hsync;
    wire       video_vsync;
    wire       video_newframe;
    wire       video_oddline;

    wire [7:0] rddata_rom;
    wire [7:0] rddata_ram;
    wire [7:0] rddata_espctrl;
    wire [7:0] rddata_espdata;
    wire [7:0] rddata_io_video;

    wire [7:0] video_vcnt;
    wire [7:0] video_hcnt;
    wire       video_irq;

    reg  [4:0] q_reg_bank0;
    reg  [4:0] q_reg_bank1;
    reg  [4:0] q_reg_bank2;
    reg  [7:0] q_reg_ramctrl;

    wire       force_turbo;
    reg        q_sysctrl_turbo           = 1'b0;
    reg        q_sysctrl_turbo_unlimited = 1'b0;

    assign turbo           = force_turbo || q_sysctrl_turbo;
    assign turbo_unlimited = force_turbo || q_sysctrl_turbo_unlimited;

    wire       spi_reset_req;

    reg q_startup_mode;

    assign reset_req = spi_reset_req || (hctrl1_data == 8'b00011101);   // Button 1+3+5

    //////////////////////////////////////////////////////////////////////////
    // Bus interface
    //////////////////////////////////////////////////////////////////////////

    // Select banking register based on upper address bits
    reg [4:0] reg_bank;
    always @* begin
        if      (ebus_a[15:14] == 2'b00 && ebus_a >= 16'h400) reg_bank = q_reg_bank0;
        else if (ebus_a[15:14] == 2'b01)                      reg_bank = q_reg_bank1;
        else if (ebus_a[15:14] == 2'b10)                      reg_bank = q_reg_bank2;
        else                                                  reg_bank = 5'b0;
    end

    wire [7:0] wrdata;

    wire bus_read_done;
    wire bus_write_done;
    wire bus_read;
    wire bus_write;
    wire bus_read2;
    wire bus_write2;

    // Memory space decoding
    wire sel_mem_rom     = q_startup_mode && mreq && ebus_a[15:14] == 2'b00;
    wire sel_mem_intram  = mreq && ebus_a[15:14] == 2'b11;

    wire sel_mem_bank2   = mreq && ebus_a == 16'hFFFF;
    wire sel_mem_bank1   = mreq && ebus_a == 16'hFFFE;
    wire sel_mem_bank0   = mreq && ebus_a == 16'hFFFD;
    wire sel_mem_ramctrl = mreq && ebus_a == 16'hFFFC;

    assign ebus_ba = reg_bank;

    // IO space decoding
    wire [2:0] io_addr         = {ebus_a[7], ebus_a[6], ebus_a[0]};
    wire       sel_io_espctrl  = q_startup_mode && iorq && ebus_a[7:0] == 8'h10;
    wire       sel_io_espdata  = q_startup_mode && iorq && ebus_a[7:0] == 8'h11;
    wire       sel_8bit        = sel_io_espctrl | sel_io_espdata;

    wire       sel_io_3f       = !sel_8bit && iorq && io_addr == 3'b001;
    wire       sel_io_vcnt     = !sel_8bit && iorq && io_addr == 3'b010;
    wire       sel_io_hcnt     = !sel_8bit && iorq && io_addr == 3'b011;
    wire       sel_io_psg      = !sel_8bit && iorq && io_addr == 3'b011;
    wire       sel_io_vdp_data = !sel_8bit && iorq && io_addr == 3'b100;
    wire       sel_io_vdp_ctrl = !sel_8bit && iorq && io_addr == 3'b101;
    wire       sel_io_dc       = !sel_8bit && iorq && io_addr == 3'b110;
    wire       sel_io_dd       = !sel_8bit && iorq && io_addr == 3'b111;

    wire       sel_internal    = iorq | sel_mem_intram | sel_mem_rom;
    wire       allow_sel_mem   = mreq && !sel_internal && (ebus_wr_n || (!ebus_wr_n && q_startup_mode));
    wire       sel_mem_ram     = allow_sel_mem;

    wire       ram_wren        = sel_mem_intram && bus_write;
    wire       io_video_wren   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_write;
    wire       io_video_rden   = (sel_io_vdp_data || sel_io_vdp_ctrl) && bus_read;

    wire       io_psg_wren     = sel_io_psg && bus_write;

    // Generate rddone signal for video
    reg io_video_rddone;
    reg q_io_video_reading;
    always @(posedge clk) begin
        io_video_rddone <= 1'b0;
        if (io_video_rden) q_io_video_reading <= 1'b1;
        if (q_io_video_reading && bus_read_done) begin
            q_io_video_reading <= 1'b0;
            io_video_rddone <= 1'b1;
        end
    end

    // Generate wrdone signal for video
    reg io_video_wrdone, q_io_video_writing;
    always @(posedge clk) begin
        io_video_wrdone <= 1'b0;
        if (io_video_wren) q_io_video_writing <= 1'b1;
        if (q_io_video_writing && bus_write_done) begin
            q_io_video_writing <= 1'b0;
            io_video_wrdone <= 1'b1;
        end
    end

    // Handle region detection at port $3F
    reg [1:0] q_region_bits;
    always @(posedge clk or posedge reset)
        if (reset)                       q_region_bits <= 2'b11;
        else if (sel_io_3f && bus_write) q_region_bits <= {wrdata[7], wrdata[5]};

    wire [7:0] port_dc, port_dd;

    reg [7:0] rddata;
    always @* begin
        rddata = 8'hFF;
        if (sel_mem_rom)              rddata = rddata_rom;
        if (sel_mem_intram)           rddata = rddata_ram;

        if (sel_io_espctrl)           rddata = rddata_espctrl;
        if (sel_io_espdata)           rddata = rddata_espdata;
        if (sel_io_vcnt)              rddata = video_vcnt;
        if (sel_io_hcnt)              rddata = video_hcnt;
        if (sel_io_vdp_data)          rddata = rddata_io_video;
        if (sel_io_vdp_ctrl)          rddata = rddata_io_video;
        if (sel_io_dc)                rddata = port_dc;
        if (sel_io_dd)                rddata = port_dd;
    end

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_startup_mode <= 1'b1;
            q_reg_bank0    <= 5'd0;
            q_reg_bank1    <= 5'd1;
            q_reg_bank2    <= 5'd2;
            q_reg_ramctrl  <= 8'd0;

        end else begin
            if (sel_mem_ramctrl && bus_write) begin
                q_startup_mode <= 1'b0;
                q_reg_ramctrl  <= wrdata;
            end
            if (sel_mem_bank0 && bus_write) q_reg_bank0 <= wrdata[4:0];
            if (sel_mem_bank1 && bus_write) q_reg_bank1 <= wrdata[4:0];
            if (sel_mem_bank2 && bus_write) q_reg_bank2 <= wrdata[4:0];
        end
    end

    assign cassette_out   = 1'b0;
    assign printer_out    = 1'b1;

    //////////////////////////////////////////////////////////////////////////
    // System ROM
    //////////////////////////////////////////////////////////////////////////
    rom rom(
        .clk    ( clk          ),
        .addr   ( ebus_a[12:0] ),
        .rddata ( rddata_rom   )
    );

    //////////////////////////////////////////////////////////////////////////
    // System RAM
    //////////////////////////////////////////////////////////////////////////
    ram ram(
        .clk    ( clk          ),
        .addr   ( ebus_a[12:0] ),
        .rddata ( rddata_ram   ),
        .wrdata ( wrdata       ),
        .wren   ( ram_wren     )
    );

    //////////////////////////////////////////////////////////////////////////
    // ESP32 UART
    //////////////////////////////////////////////////////////////////////////
    wire [8:0] esp_tx_data;
    wire       esp_tx_wr;
    wire       esp_tx_fifo_full;
    wire [8:0] esp_rx_data;
    wire       esp_rx_rd;
    wire       esp_rx_empty;
    wire       esp_rx_fifo_overflow;
    wire       esp_rx_framing_error;

    aqp_esp_uart esp_uart(
        .clk              ( clk                  ),
        .reset            ( reset                ),

        .txfifo_data      ( esp_tx_data          ),
        .txfifo_wr        ( esp_tx_wr            ),
        .txfifo_full      ( esp_tx_fifo_full     ),

        .rxfifo_data      ( esp_rx_data          ),
        .rxfifo_rd        ( esp_rx_rd            ),
        .rxfifo_empty     ( esp_rx_empty         ),
        .rxfifo_overflow  ( esp_rx_fifo_overflow ),
        .rx_framing_error ( esp_rx_framing_error ),

        .esp_rx           ( esp_rx               ),
        .esp_tx           ( esp_tx               ),
        .esp_cts          ( esp_cts              ),
        .esp_rts          ( esp_rts              )
    );

    assign esp_tx_data = sel_io_espctrl ? 9'b100000000 : {1'b0, wrdata};
    assign esp_tx_wr   = bus_write2 && (sel_io_espdata || (sel_io_espctrl && wrdata[7]));
    assign esp_rx_rd   = bus_read2  &&  sel_io_espdata;

    reg q_esp_rx_fifo_overflow, q_esp_rx_framing_error;

    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_esp_rx_fifo_overflow <= 1'b0;
            q_esp_rx_framing_error <= 1'b0;
        end else begin
            if (sel_io_espctrl && bus_write2) begin
                q_esp_rx_fifo_overflow <= q_esp_rx_fifo_overflow & ~wrdata[4];
                q_esp_rx_framing_error <= q_esp_rx_framing_error & ~wrdata[3];
            end

            if (esp_rx_fifo_overflow) q_esp_rx_fifo_overflow <= 1'b1;
            if (esp_rx_framing_error) q_esp_rx_framing_error <= 1'b1;
        end
    end

    assign rddata_espctrl = {3'b0, q_esp_rx_fifo_overflow, q_esp_rx_framing_error, esp_rx_data[8], esp_tx_fifo_full, !esp_rx_empty};
    assign rddata_espdata = esp_rx_data[7:0];

    //////////////////////////////////////////////////////////////////////////
    // Video
    //////////////////////////////////////////////////////////////////////////
    video video(
        .clk            ( clk             ),
        .reset          ( reset           ),

        .vclk           ( video_clk       ),

        .io_portsel     ( ebus_a[0]       ),
        .io_rddata      ( rddata_io_video ),
        .io_wrdata      ( wrdata          ),
        .io_wren        ( io_video_wren   ),
        .io_wrdone      ( io_video_wrdone ),
        .io_rddone      ( io_video_rddone ),
        .irq            ( video_irq       ),

        .vcnt           ( video_vcnt      ),
        .hcnt           ( video_hcnt      ),

        .video_r        ( video_r         ),
        .video_g        ( video_g         ),
        .video_b        ( video_b         ),
        .video_de       ( video_de        ),
        .video_hsync    ( video_hsync     ),
        .video_vsync    ( video_vsync     ),
        .video_newframe ( video_newframe  ),
        .video_oddline  ( video_oddline   )
    );

    //////////////////////////////////////////////////////////////////////////
    // Hand controller interface
    //////////////////////////////////////////////////////////////////////////
    wire  [7:0] hc1_in = hc1[7:0];
    wire  [7:0] hc1_out;
    wire        hc1_oe;

    wire  [7:0] hc2_in = hc2[7:0];
    wire  [7:0] hc2_out;
    wire        hc2_oe;

    assign hc1[7:0] = hc1_oe ? hc1_out : 8'bZ;
    assign hc2[7:0] = hc2_oe ? hc2_out : 8'bZ;
    assign hc1[8]   = 1'b0;
    assign hc2[8]   = 1'b0;

    wire [7:0] spi_hctrl1, spi_hctrl2;

    wire [7:0] hctrl1 = hc1_in[7:0];
    wire [7:0] hctrl2 = hc2_in[7:0];

    // Synchronize inputs
    reg [7:0] q_hctrl1, q2_hctrl1;
    reg [7:0] q_hctrl2, q2_hctrl2;
    always @(posedge clk) q_hctrl1  <= hctrl1;
    always @(posedge clk) q2_hctrl1 <= q_hctrl1;
    always @(posedge clk) q_hctrl2  <= hctrl2;
    always @(posedge clk) q2_hctrl2 <= q_hctrl2;

    // Combine data from ESP with data from handcontroller input
    wire [7:0] hctrl1_data = q2_hctrl1 & spi_hctrl1;
    wire [7:0] hctrl2_data = q2_hctrl2 & spi_hctrl2;

    //////////////////////////////////////////////////////////////////////////
    // SPI interface
    //////////////////////////////////////////////////////////////////////////
    wire [63:0] keys;

    spiregs spiregs(
        .clk              ( clk              ),
        .reset            ( reset            ),

        .spi_msg_end      ( spi_msg_end      ),
        .spi_cmd          ( spi_cmd          ),
        .spi_rxdata       ( spi_rxdata       ),
        .spi_txdata       ( spi_txdata       ),
        .spi_txdata_valid ( spi_txdata_valid ),

        .reset_req        ( spi_reset_req    ),
        .keys             ( keys             ),
        .hctrl1           ( spi_hctrl1       ),
        .hctrl2           ( spi_hctrl2       ),

        .use_t80          ( use_t80          ),
        .has_z80          ( has_z80          ),
        .force_turbo      ( force_turbo      )
    );

    assign video_mode = 1;

    //////////////////////////////////////////////////////////////////////////
    // Keyboard
    //////////////////////////////////////////////////////////////////////////
    wire joypad_a_tr    = keys[45] && hctrl1_data[6]; // X
    wire joypad_a_tl    = keys[51] && hctrl1_data[5]; // Z
    wire joypad_a_right = keys[15] && hctrl1_data[1];
    wire joypad_a_left  = keys[22] && hctrl1_data[3];
    wire joypad_a_down  = keys[23] && hctrl1_data[0];
    wire joypad_a_up    = keys[14] && hctrl1_data[2];

    wire joypad_b_tr    = hctrl2_data[6];
    wire joypad_b_tl    = hctrl2_data[5];
    wire joypad_b_right = hctrl2_data[1];
    wire joypad_b_left  = hctrl2_data[3];
    wire joypad_b_down  = hctrl2_data[0];
    wire joypad_b_up    = hctrl2_data[2];

    assign port_dc = {joypad_b_down, joypad_b_up, joypad_a_tr, joypad_a_tl, joypad_a_right, joypad_a_left, joypad_a_down, joypad_a_up};
    assign port_dd = {q_region_bits, 2'b11, joypad_b_tr, joypad_b_tl, joypad_b_right, joypad_b_left};

    //////////////////////////////////////////////////////////////////////////
    // SN76489 PSG
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] psg_sample;

    psg psg(
        .clk    ( clk         ),
        .reset  ( reset       ),

        .wrdata ( wrdata      ),
        .wren   ( io_psg_wren ),

        .sample ( psg_sample  )
    );

    always @(posedge clk) common_audio_l <= {~psg_sample[15], psg_sample[14:0]};
    always @(posedge clk) common_audio_r <= {~psg_sample[15], psg_sample[14:0]};

    assign hc1_out = 8'b0;
    assign hc1_oe  = 1'b0;
    assign hc2_out = 8'b0;
    assign hc2_oe  = 1'b0;

    //////////////////////////////////////////////////////////////////////////
    // Display overlay
    //////////////////////////////////////////////////////////////////////////
    aqp_overlay overlay(
        // Core video interface
        .video_clk          ( video_clk          ),
        .video_r            ( video_r            ),
        .video_g            ( video_g            ),
        .video_b            ( video_b            ),
        .video_de           ( video_de           ),
        .video_hsync        ( video_hsync        ),
        .video_vsync        ( video_vsync        ),
        .video_newframe     ( video_newframe     ),
        .video_oddline      ( video_oddline      ),
        .video_mode         ( video_mode         ),

        // Overlay interface
        .ovl_clk            ( clk                ),

        .ovl_text_addr      ( ovl_text_addr      ),
        .ovl_text_wrdata    ( ovl_text_wrdata    ),
        .ovl_text_wr        ( ovl_text_wr        ),

        .ovl_font_addr      ( ovl_font_addr      ),
        .ovl_font_wrdata    ( ovl_font_wrdata    ),
        .ovl_font_wr        ( ovl_font_wr        ),

        .ovl_palette_addr   ( ovl_palette_addr   ),
        .ovl_palette_wrdata ( ovl_palette_wrdata ),
        .ovl_palette_wr     ( ovl_palette_wr     ),

        // VGA signals
        .vga_r              ( vga_r              ),
        .vga_g              ( vga_g              ),
        .vga_b              ( vga_b              ),
        .vga_hsync          ( vga_hsync          ),
        .vga_vsync          ( vga_vsync          )
    );

    //////////////////////////////////////////////////////////////////////////
    // T80 core
    //////////////////////////////////////////////////////////////////////////
    wire [15:0] t80_addr;
    wire  [7:0] t80_dq_out;
    wire  [7:0] t80_dq_in = ebus_d;
    wire        t80_dq_oe;

    wire        t80_mreq_n;
    wire        t80_iorq_n;
    wire        t80_rd_n;
    wire        t80_wr_n;
    wire        t80_wait_n = 1'b1;

    wire        t80_busrq_n = spibm_busreq_n;
    wire        t80_busak_n;

    wire        t80_int_n;
    wire        t80_nmi_n = 1'b1;

    aqp_t80 aqp_t80(
        .clk     ( clk               ),
        .reset   ( reset || !use_t80 ),
        .clken   ( ebus_phi_clken    ),
        .phi     ( ebus_phi          ),

        .addr    ( t80_addr          ),
        .dq_out  ( t80_dq_out        ),
        .dq_in   ( t80_dq_in         ),
        .dq_oe   ( t80_dq_oe         ),

        .mreq_n  ( t80_mreq_n        ),
        .iorq_n  ( t80_iorq_n        ),
        .rd_n    ( t80_rd_n          ),
        .wr_n    ( t80_wr_n          ),
        .wait_n  ( t80_wait_n        ),

        .busrq_n ( t80_busrq_n       ),
        .busak_n ( t80_busak_n       ),

        .int_n   ( t80_int_n         ),
        .nmi_n   ( t80_nmi_n         )
    );

    //////////////////////////////////////////////////////////////////////////
    // Bus logic
    //////////////////////////////////////////////////////////////////////////
    wire   spibm_en = !spibm_busreq_n && (!ebus_busack_n || !has_z80);

    assign ebus_a = spibm_en ? spibm_a : (use_t80 ? t80_addr : 16'bZ);
    assign ebus_d =
        (spibm_en && spibm_wrdata_en) ? spibm_wrdata :
        (use_t80 && t80_dq_oe         ? t80_dq_out   :
        (!ebus_rd_n && sel_internal   ? rddata       :
                                        8'bZ));
    assign ebus_rd_n      = spibm_en ? spibm_rd_n   : (use_t80 ? t80_rd_n   : 1'bZ);
    assign ebus_wr_n      = spibm_en ? spibm_wr_n   : (use_t80 ? t80_wr_n   : 1'bZ);
    assign ebus_mreq_n    = spibm_en ? spibm_mreq_n : (use_t80 ? t80_mreq_n : 1'bZ);
    assign ebus_iorq_n    = spibm_en ? spibm_iorq_n : (use_t80 ? t80_iorq_n : 1'bZ);
    assign ebus_busreq_n  = !(use_t80 || !spibm_busreq_n);
    assign ebus_ram_ce_n  = !sel_mem_ram;
    assign ebus_cart_ce_n = 1;
    assign ebus_ram_we_n  = !(!ebus_wr_n && sel_mem_ram && q_startup_mode);

    wire   ebus_int_n_pushpull = video_irq ? 1'b0 : 1'b1;
    assign ebus_int_n = !ebus_int_n_pushpull ? 1'b0 : 1'bZ;

    assign t80_int_n = ebus_int_n_pushpull;

    // Synchronize RD#/WR# signals for when using external Z80
    reg [2:0] q_ebus_wr_n;
    reg [2:0] q_ebus_rd_n;
    always @(posedge clk) q_ebus_wr_n <= {q_ebus_wr_n[1:0], ebus_wr_n};
    always @(posedge clk) q_ebus_rd_n <= {q_ebus_rd_n[1:0], ebus_rd_n};

    // Register data from external bus
    reg [7:0] ebus_d_in;
    always @(posedge clk) if (!ebus_wr_n) ebus_d_in <= ebus_d;

    assign bus_read       = q_ebus_rd_n[2:1] == 2'b10;
    assign bus_read_done  = q_ebus_rd_n[2:1] == 2'b01;
    assign bus_write      = q_ebus_wr_n[2:1] == 2'b10;
    assign bus_write_done = q_ebus_wr_n[2:1] == 2'b01;

    assign wrdata     = ebus_d_in;
    assign bus_read2  = bus_read;
    assign bus_write2 = bus_write;
    assign iorq       = !ebus_iorq_n;
    assign mreq       = !ebus_mreq_n;

endmodule
