`default_nettype none
`timescale 1 ns / 1 ps

module pcm(
    input  wire        clk,
    input  wire        reset,

    input  wire  [1:0] bus_addr,
    input  wire [31:0] bus_wrdata,
    input  wire        bus_wren,
    output reg  [31:0] bus_rddata,

    output wire        irq,

    output reg  [15:0] audio_l,
    output reg  [15:0] audio_r
);

    wire [31:0] fifo_rddata;
    wire        fifo_empty;
    wire        fifo_full;
    wire  [9:0] fifo_count;

    reg   [9:0] q_threshold;
    reg  [11:0] q_rate;

    wire fifo_reset = (bus_wren && bus_addr == 2'd0) || reset;
    wire fifo_wr    = (bus_wren && bus_addr == 2'd3);

    always @* case (bus_addr)
        default: bus_rddata = {22'b0, fifo_count};
        2'd1:    bus_rddata = {22'b0, q_threshold};
        2'd2:    bus_rddata = {20'b0, q_rate};
    endcase

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_threshold <= 0;
            q_rate      <= 0;

        end else begin
            if (bus_wren) begin
                if (bus_addr == 2'd1) q_threshold <= bus_wrdata[9:0];
                if (bus_addr == 2'd2) q_rate      <= bus_wrdata[11:0];
            end
        end

    assign irq = fifo_count < q_threshold;

    reg [19:0] q_phase_accum;
    reg        q_next_sample;

    wire [20:0] d_phase_accum = {1'b0, q_phase_accum} + {9'b0, q_rate};
    always @(posedge clk or posedge reset) begin
        if (reset) begin
            q_phase_accum <= 0;
            q_next_sample <= 0;
        end else begin
            q_phase_accum <= d_phase_accum[19:0];
            q_next_sample <= d_phase_accum[20];
        end
    end

    pcm_fifo pcm_fifo(
        .clk(clk),
        .reset(fifo_reset),

        .wrdata(bus_wrdata),
        .wr_en(fifo_wr),

        .rddata(fifo_rddata),
        .rd_en(q_next_sample),
        
        .empty(fifo_empty),
        .full(fifo_full),
        .count(fifo_count));

    always @(posedge clk) begin
        audio_l <= fifo_empty ? 16'b0 : fifo_rddata[15:0];
        audio_r <= fifo_empty ? 16'b0 : fifo_rddata[31:16];
    end

endmodule
