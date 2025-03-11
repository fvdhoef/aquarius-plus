`default_nettype none
`timescale 1 ns / 1 ps

module sram_ctrl(
    input  wire        clk,
    input  wire        reset,

    // Command interface
    input  wire [16:0] bus_addr,
    input  wire [31:0] bus_wrdata,
    input  wire  [3:0] bus_bytesel,
    input  wire        bus_wren,
    input  wire        bus_strobe,
    output wire        bus_wait,
    output wire [31:0] bus_rddata,

    // SRAM interface
    output reg  [18:0] sram_a,
    output wire        sram_ce_n,
    output reg         sram_oe_n,
    output reg         sram_we_n,
    inout  wire  [7:0] sram_dq
);

    assign sram_ce_n = 1'b0;

    reg  [2:0] d_state,      q_state;
    reg [18:0] d_sram_a,     q_sram_a;
    reg        d_sram_we_n,  q_sram_we_n;
    reg        d_sram_oe_n,  q_sram_oe_n;
    reg        d_bus_wait,   q_bus_wait;
    reg [31:0] d_bus_rddata, q_bus_rddata;
    reg  [7:0] d_dq_wrdata,  q_dq_wrdata;
    reg        d_dq_oe,      q_dq_oe;

    assign sram_dq = q_dq_oe ? q_dq_wrdata : 8'bZ;

    assign bus_wait   = q_bus_wait;
    assign bus_rddata = q_bus_rddata;

    localparam [2:0]
        StIdle   = 3'd0,
        StRead   = 3'd1,
        StWrite  = 3'd2,
        StWrite2 = 3'd3;

    always @* begin
        d_state      = q_state;
        d_sram_a     = q_sram_a;
        d_sram_we_n  = q_sram_we_n;
        d_sram_oe_n  = q_sram_oe_n;
        d_bus_wait   = 1;
        d_bus_rddata = q_bus_rddata;
        d_dq_wrdata  = q_dq_wrdata;
        d_dq_oe      = q_dq_oe;

        case (q_state)
            StIdle: begin
                d_sram_we_n = 1;
                d_sram_oe_n = 1;
                d_dq_oe     = 0;

                if (bus_strobe && q_bus_wait) begin
                    if (bus_wren) begin
                        d_state     = StWrite;
                        d_sram_a    = {bus_addr, 2'b11};
                        d_sram_oe_n = 1;
                    end else begin
                        d_state     = StRead;
                        d_sram_a    = {bus_addr, 2'b00};
                        d_sram_we_n = 1;
                        d_sram_oe_n = 0;
                    end
                end
            end

            StRead: begin
                d_sram_a[1:0] = q_sram_a[1:0] + 2'd1;

                case (q_sram_a[1:0])
                    2'd0: d_bus_rddata[ 7: 0] = sram_dq;
                    2'd1: d_bus_rddata[15: 8] = sram_dq;
                    2'd2: d_bus_rddata[23:16] = sram_dq;
                    2'd3: d_bus_rddata[31:24] = sram_dq;
                endcase

                if (q_sram_a[1:0] == 2'd3) begin
                    d_sram_oe_n = 0;
                    d_bus_wait  = 0;
                    d_state     = StIdle;
                end
            end

            StWrite: begin
                d_sram_a[1:0] = q_sram_a[1:0] + 2'd1;
                d_dq_oe       = 1;
                case (d_sram_a[1:0])
                    2'd0: begin d_dq_wrdata = bus_wrdata[ 7: 0]; d_sram_we_n = !bus_bytesel[0]; end
                    2'd1: begin d_dq_wrdata = bus_wrdata[15: 8]; d_sram_we_n = !bus_bytesel[1]; end
                    2'd2: begin d_dq_wrdata = bus_wrdata[23:16]; d_sram_we_n = !bus_bytesel[2]; end
                    2'd3: begin d_dq_wrdata = bus_wrdata[31:24]; d_sram_we_n = !bus_bytesel[3]; end
                endcase

                if (!d_sram_we_n)
                    d_state = StWrite2;
                else if (d_sram_a[1:0] == 2'd3) begin
                    d_bus_wait = 0;
                    d_state    = StIdle;
                end
            end

            StWrite2: begin
                d_sram_we_n = 1;

                if (q_sram_a[1:0] == 2'd3) begin
                    d_bus_wait = 0;
                    d_state    = StIdle;
                end else begin
                    d_state    = StWrite;
                end
            end

            default: d_state = StIdle;
        endcase
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_state      <= StIdle;
            q_sram_a     <= 0;
            q_sram_we_n  <= 1;
            q_sram_oe_n  <= 1;
            q_bus_wait   <= 1;
            q_bus_rddata <= 0;
            q_dq_wrdata  <= 0;
            q_dq_oe      <= 0;
        end else begin
            q_state      <= d_state;
            q_sram_a     <= d_sram_a;
            q_sram_we_n  <= d_sram_we_n;
            q_sram_oe_n  <= d_sram_oe_n;
            q_bus_wait   <= d_bus_wait;
            q_bus_rddata <= d_bus_rddata;
            q_dq_wrdata  <= d_dq_wrdata;
            q_dq_oe      <= d_dq_oe;
        end

    always @(posedge clk) sram_a    <= d_sram_a;
    always @(posedge clk) sram_we_n <= d_sram_we_n;
    always @(posedge clk) sram_oe_n <= d_sram_oe_n;

endmodule
