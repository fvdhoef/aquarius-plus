`timescale 1 ns / 1 ps

module tb_fmsynth();

    reg clk   = 0;
    reg reset = 1;

    always #20 clk = !clk;
    always #200 reset = 0;

    wire [15:0] audio_l;
    wire [15:0] audio_r;

    reg   [7:0] bus_addr;
    reg  [31:0] bus_wrdata;
    reg         bus_wren;
    wire [31:0] bus_rddata;
    wire        bus_wait;

    fmsynth fmsynth(
        .clk(clk),
        .reset(reset),

        .bus_addr(bus_addr),
        .bus_wrdata(bus_wrdata),
        .bus_wren(bus_wren),
        .bus_rddata(bus_rddata),
        .bus_wait(bus_wait),

        .audio_l(audio_l),
        .audio_r(audio_r)
    );

    task regwr;
        input  [7:0] addr;
        input [31:0] data;

        begin
            bus_addr    = addr;
            bus_wrdata  = data;
            bus_wren    = 1;
            @(posedge clk);
            while (bus_wait) @(posedge clk);
            bus_wren  = 0;
        end
    endtask

    initial begin
        bus_addr   = 0;
        bus_wrdata = 0;
        bus_wren   = 0;

        @(negedge(reset));
        @(posedge(clk));

        regwr(8'h80, {
            1'b0,   // AM
            1'b0,   // VIB
            1'b0,   // SUS
            1'b0,   // KSR
            4'd1,   // MULT
            2'd0,   // KSL
            6'd0,   // TL
            4'd5,   // AR
            4'd0,   // DR
            4'd15,  // SL
            4'd7    // RR
        });
        regwr(8'h81, {
            1'b0,   // AM
            1'b0,   // VIB
            1'b0,   // SUS
            1'b0,   // KSR
            4'd1,   // MULT
            2'd0,   // KSL
            6'd0,   // TL
            4'd5,   // AR
            4'd0,   // DR
            4'd15,  // SL
            4'd7    // RR
        });
        // regwr(8'h81, {
        //     1'b0,   // AM
        //     1'b0,   // VIB
        //     1'b0,   // SUS
        //     1'b0,   // KSR
        //     4'd1,   // MULT
        //     2'd0,   // KSL
        //     6'd63,  // TL
        //     4'd15,  // AR
        //     4'd0,   // DR
        //     4'd15,  // SL
        //     4'd7    // RR
        // });
        regwr(8'h60, {
            10'b0,  // -
            1'b1,   // CHB
            1'b1,   // CHA
            3'd4,   // FB
            1'b1,   // CNT
            2'b0,   // -
            1'b1,   // KON
            3'd3,   // BLOCK
            10'd344 // FNUM
        });

        regwr(8'h02, 32'h1);
        
        // #1377900;
        // @(posedge(clk));

        // regwr(8'h60, {
        //     10'b0,  // -
        //     1'b1,   // CHB
        //     1'b1,   // CHA
        //     3'd0,   // FB
        //     1'b0,   // CNT
        //     2'b0,   // -
        //     1'b0,   // KON
        //     3'd0,   // BLOCK
        //     10'd300 // FNUM
        // });
        // regwr(8'h60, {
        //     10'b0,  // -
        //     1'b1,   // CHB
        //     1'b1,   // CHA
        //     3'd0,   // FB
        //     1'b0,   // CNT
        //     2'b0,   // -
        //     1'b1,   // KON
        //     3'd0,   // BLOCK
        //     10'd300 // FNUM
        // });

    end

endmodule
