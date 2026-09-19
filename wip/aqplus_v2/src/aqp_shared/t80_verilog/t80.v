`default_nettype none
`timescale 1 ns / 1 ps

module t80(
    input  wire        clk,
    input  wire        reset,

    input  wire        clk_en,

    output wire [15:0] bus_addr,
    output wire  [7:0] bus_wrdata,
    output wire        bus_wren,
    output wire        bus_iorq,
    output wire        bus_strobe,
    input  wire        bus_wait,
    input  wire  [7:0] bus_rddata,

    input  wire        irq,
    input  wire  [7:0] irq_vector,
    input  wire        nmi);

`ifdef MODEL_TECH
    initial begin
        forever begin
            @(posedge clk);
            if (bus_strobe && !bus_wait) begin
                if (bus_iorq) begin
                    if (bus_wren)
                        $display("%0t IO  WR   %02h=%02h", $time, bus_addr[7:0], bus_wrdata);
                    else
                        $display("%0t IO  RD   %02h:%02h", $time, bus_addr[7:0], bus_rddata);
                end else begin
                    if (bus_wren)
                        $display("%0t MEM WR %04h=%02h", $time, bus_addr, bus_wrdata);
                    else
                        $display("%0t MEM RD %04h:%02h", $time, bus_addr, bus_rddata);
                end
            end
        end
    end
`endif

    localparam
        Flag_C = 0,
        Flag_N = 1,
        Flag_P = 2,
        Flag_X = 3,
        Flag_H = 4,
        Flag_Y = 5,
        Flag_Z = 6,
        Flag_S = 7;

    localparam
        aNone = 3'd7,
        aBC   = 3'd0,
        aDE   = 3'd1,
        aXY   = 3'd2,
        aIOA  = 3'd4,
        aSP   = 3'd5,
        aZI   = 3'd6;

    localparam
        PrefixNone  = 2'b00,
        PrefixCB    = 2'b01,
        PrefixED    = 2'b10,
        PrefixDD_FD = 2'b11;

    localparam
        AluOpAdd = 4'd0,
        AluOpAdc = 4'd1,
        AluOpSub = 4'd2,
        AluOpSbc = 4'd3,
        AluOpAnd = 4'd4,
        AluOpXor = 4'd5,
        AluOpOr  = 4'd6,
        AluOpCp  = 4'd7,
        AluOpRot = 4'd8,
        AluOpBit = 4'd9,
        AluOpSet = 4'd10,
        AluOpRes = 4'd11,
        AluOpDaa = 4'd12,
        AluOpRld = 4'd13,
        AluOpRrd = 4'd14;

    // Registers
    reg  [15:0] q_reg_pc;       // PC  program counter
    reg   [7:0] q_reg_a;        // A
    reg   [7:0] q_reg_f;        // F   flags
    reg   [7:0] q_reg_a_alt;    // A'
    reg   [7:0] q_reg_f_alt;    // F'
    reg   [7:0] q_reg_i;        // I   interrupt vector register
    reg   [7:0] q_reg_r;        // R   refresh register
    reg  [15:0] q_reg_sp;       // SP  stack pointer
    reg  [15:0] q_bus_addr;
    reg   [7:0] q_bus_wrdata;

    wire [15:0] reg_bus_a;
    wire [15:0] reg_bus_b;
    wire [15:0] reg_bus_c;
    reg         reg_wren_h;
    reg         reg_wren_l;

    reg   [7:0] bus_b;
    reg   [7:0] bus_a;

    wire        d_auto_wait;

    reg   [3:0] q_alu_op;
    reg         q_regs_alt;     // Use alternate registers
    reg  [15:0] q_memptr;
    reg   [7:0] q_instruction;  // Current instruction
    reg   [1:0] q_prefix;       // Current prefix
    reg  [15:0] q_reg_bus_a;
    reg   [2:0] q_tstate;
    reg   [2:0] q_mcycle;
    reg         q_int_en1;
    reg         q_int_en2;
    reg         q_halt;
    reg   [1:0] q_xy_state;
    reg   [1:0] q_im;           // Interrupt mode
    reg         q_no_btr;
    reg         q_btr;
    reg         q_auto_wait;
    reg         q_inc_dec_is_zero;
    reg         q_arith16;
    reg         q_z16;
    reg         q_save_alu;
    reg         q_preserve_c;
    reg   [2:0] q_mcycles;
    reg         q_irq_cycle;
    reg         q_nmi_cycle;
    reg   [4:0] q_read_to_reg;
    reg         q_xy_ind;

    wire [15:0] incdec16_result;

    reg  [7:0] q_di;

    reg       dec_no_read;
    reg       dec_write;
    reg       dec_iorq;

    //------------------------------------------------------------------------
    // Bus strobe
    //------------------------------------------------------------------------
    reg       t80_wait;
    reg       d_strobe, q_strobe;
    reg       d_wren,   q_wren;
    reg       d_iorq,   q_iorq;
    reg [7:0] d_rddata, q_rddata;

    always @* begin
        t80_wait = 1;
        d_strobe = q_strobe;
        d_rddata = q_rddata;
        d_wren   = q_wren;
        d_iorq   = q_iorq;

        if (q_strobe && !bus_wait) begin
            d_strobe = 0;
            t80_wait = 0;
            d_rddata = bus_rddata;
        end

        if (q_mcycle == 1) begin
            d_wren   = 0;
            d_iorq   = 0;

            if (q_tstate == 1 && !q_irq_cycle) begin
                d_strobe = 1;
            end
        end else begin
            d_wren   = dec_write;
            d_iorq   = dec_iorq;

            if (q_tstate == 1 && (dec_write || (!dec_write && !dec_no_read))) begin
                d_strobe = 1;
            end
        end

    end

    always @(posedge clk) begin
        q_strobe <= d_strobe;
        q_wren   <= d_wren;
        q_iorq   <= d_iorq;
        q_rddata <= d_rddata;

        if (clk_en && q_tstate == 3'd2) q_di <= d_rddata;

        if (reset) begin
            q_strobe <= 0;
        end
    end

    assign bus_addr   = q_bus_addr;
    assign bus_wrdata = q_bus_wrdata;
    assign bus_wren   = q_wren;
    assign bus_iorq   = q_iorq;
    assign bus_strobe = q_strobe;

    //------------------------------------------------------------------------
    // Instruction decoder and sequencer
    //------------------------------------------------------------------------
    reg cc_is_true;
    always @* begin
        case (q_instruction[5:3])
            3'd0:    cc_is_true = !q_reg_f[Flag_Z]; // NZ
            3'd1:    cc_is_true =  q_reg_f[Flag_Z]; // Z
            3'd2:    cc_is_true = !q_reg_f[Flag_C]; // NC
            3'd3:    cc_is_true =  q_reg_f[Flag_C]; // C
            3'd4:    cc_is_true = !q_reg_f[Flag_P]; // PO
            3'd5:    cc_is_true =  q_reg_f[Flag_P]; // PE
            3'd6:    cc_is_true = !q_reg_f[Flag_S]; // P
            default: cc_is_true =  q_reg_f[Flag_S]; // M
        endcase
    end

    wire [2:0] ir_ddd   = q_instruction[5:3];
    wire [2:0] ir_sss   = q_instruction[2:0];
    wire [1:0] ir_dpair = q_instruction[5:4];

    reg       dec_arith16;
    reg       dec_call;
    reg       dec_exchange_rp;
    reg       dec_exchange_wh;
    reg       dec_inc_memptr;
    reg       dec_inc_pc;
    reg       dec_is_bc;
    reg       dec_is_bt;
    reg       dec_is_btr;
    reg       dec_is_ccf;
    reg       dec_is_cpl;
    reg       dec_is_di;
    reg       dec_is_djnz;
    reg       dec_is_ei;
    reg       dec_is_ex_af;
    reg       dec_is_ex_de_hl;
    reg       dec_is_exx;
    reg       dec_is_halt;
    reg       dec_is_inrc;
    reg       dec_is_jp_ind_hl;
    reg       dec_is_ldsphl;
    reg       dec_is_retn;
    reg       dec_is_rld;
    reg       dec_is_rrd;
    reg       dec_is_scf;
    reg       dec_jump_e;
    reg       dec_jump;
    reg       dec_ldw;
    reg       dec_ldz;
    reg       dec_no_pc;
    reg       dec_preserve_c;
    reg       dec_read_to_acc;
    reg       dec_read_to_reg;
    reg       dec_rst_p;
    reg       dec_save_alu;
    reg       dec_xybit_undoc;
    reg [1:0] dec_im;
    reg [1:0] dec_prefix;
    reg [1:0] dec_set_sw;
    reg [2:0] dec_mcycles;
    reg [2:0] dec_set_addr_to;
    reg [2:0] dec_special_ld;
    reg [2:0] dec_tstates;
    reg [3:0] dec_alu_op;
    reg [3:0] dec_incdec16;
    reg [3:0] dec_set_bus_a_to;
    reg [3:0] dec_set_bus_b_to;

    always @* begin
        dec_alu_op       = {1'b0, q_instruction[5:3]};
        dec_arith16      = 0;
        dec_call         = 0;
        dec_exchange_rp  = 0;
        dec_exchange_wh  = 0;
        dec_im           = q_im;
        dec_inc_memptr   = 0;
        dec_inc_pc       = 0;
        dec_incdec16     = 4'b0000;
        dec_iorq         = 0;
        dec_is_bc        = 0;
        dec_is_bt        = 0;
        dec_is_btr       = 0;
        dec_is_ccf       = 0;
        dec_is_cpl       = 0;
        dec_is_di        = 0;
        dec_is_djnz      = 0;
        dec_is_ei        = 0;
        dec_is_ex_af     = 0;
        dec_is_ex_de_hl  = 0;
        dec_is_exx       = 0;
        dec_is_halt      = 0;
        dec_is_inrc      = 0;
        dec_is_jp_ind_hl = 0;
        dec_is_ldsphl    = 0;
        dec_is_retn      = 0;
        dec_is_rld       = 0;
        dec_is_rrd       = 0;
        dec_is_scf       = 0;
        dec_jump         = 0;
        dec_jump_e       = 0;
        dec_ldw          = 0;
        dec_ldz          = 0;
        dec_mcycles      = 3'd1;
        dec_no_pc        = 0;
        dec_no_read      = 0;
        dec_prefix       = PrefixNone;
        dec_preserve_c   = 0;
        dec_read_to_acc  = 0;
        dec_read_to_reg  = 0;
        dec_rst_p        = 0;
        dec_save_alu     = 0;
        dec_set_addr_to  = aNone;
        dec_set_bus_a_to = 4'b0000;
        dec_set_bus_b_to = 4'b0000;
        dec_set_sw       = 2'b00;
        dec_special_ld   = 3'd0;
        dec_tstates      = (q_mcycle == 3'd1) ? 3'd4 : 3'd3;
        dec_write        = 0;
        dec_xybit_undoc  = 0;

        case (q_prefix)
            //----------------------------------------------------------------
            // Unprefixed instructions
            //----------------------------------------------------------------
            2'b00: begin
                case (q_instruction)
                    // 8 BIT LOAD GROUP
                    8'h40,8'h41,8'h42,8'h43,8'h44,8'h45,8'h47,
                    8'h48,8'h49,8'h4a,8'h4b,8'h4c,8'h4d,8'h4f,
                    8'h50,8'h51,8'h52,8'h53,8'h54,8'h55,8'h57,
                    8'h58,8'h59,8'h5a,8'h5b,8'h5c,8'h5d,8'h5f,
                    8'h60,8'h61,8'h62,8'h63,8'h64,8'h65,8'h67,
                    8'h68,8'h69,8'h6a,8'h6b,8'h6c,8'h6d,8'h6f,
                    8'h78,8'h79,8'h7a,8'h7b,8'h7c,8'h7d,8'h7f: begin    // LD r,r'
                        dec_mcycles      = 3'd1;
                        dec_set_bus_b_to = {1'b0, ir_sss};
                        dec_exchange_rp  = 1;
                        dec_set_bus_a_to = {1'b0, ir_ddd};
                        dec_read_to_reg  = 1;
                    end

                    8'h06,8'h0e,8'h16,8'h1e,8'h26,8'h2e,8'h3e: begin    // LD r,n
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_set_bus_a_to = {1'b0, ir_ddd};
                                dec_read_to_reg  = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h46,8'h4e,8'h56,8'h5e,8'h66,8'h6e,8'h7e: begin    // LD r,(HL)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_set_bus_a_to = {1'b0, ir_ddd};
                                dec_read_to_reg  = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h70,8'h71,8'h72,8'h73,8'h74,8'h75,8'h77: begin    // LD (HL),r
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                                dec_set_bus_b_to = {1'b0, ir_sss};
                            end
                            3'd2: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h36: begin    // LD (HL),n
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_set_addr_to  = aXY;
                                dec_set_bus_b_to = {1'b0, ir_sss};
                            end
                            3'd3: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h0a: begin    // LD A,(BC)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: dec_set_addr_to = aBC;
                            3'd2: dec_read_to_acc = 1;
                            default: begin end
                        endcase
                    end

                    8'h1a: begin    // LD A,(DE)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: dec_set_addr_to = aDE;
                            3'd2: dec_read_to_acc = 1;
                            default: begin end
                        endcase
                    end

                    8'h3a: begin    // LD A,(nn)
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_ldz    = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to = aZI;
                                dec_inc_pc      = 1;
                            end
                            3'd4: begin
                                dec_read_to_acc = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h02: begin    // LD (BC),A
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aBC;
                                dec_set_bus_b_to = 4'b0111;
                                dec_set_sw       = 2'b10;
                            end
                            3'd2: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h12: begin    // LD (DE),A
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aDE;
                                dec_set_bus_b_to = 4'b0111;
                                dec_set_sw       = 2'b10;
                            end
                            3'd2: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h32: begin    // LD (nn),A
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_ldz    = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to  = aZI;
                                dec_set_sw       = 2'b10;
                                dec_inc_pc       = 1;
                                dec_set_bus_b_to = 4'b0111;
                            end
                            3'd4: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    // 16 BIT LOAD GROUP
                    8'h01,8'h11,8'h21,8'h31: begin  // LD dd,nn
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (ir_dpair == 2'b11) ? 4'b1000 : {1'b0, ir_dpair, 1'b1};
                            end
                            3'd3: begin
                                dec_inc_pc       = 1;
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (ir_dpair == 2'b11) ? 4'b1001 : {1'b0, ir_dpair, 1'b0};
                            end
                            default: begin end
                        endcase
                    end

                    8'h2a: begin    // LD HL,(nn)
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_ldz          = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to  = aZI;
                                dec_inc_pc       = 1;
                                dec_ldw          = 1;
                            end
                            3'd4: begin
                                dec_set_bus_a_to = 4'b0101;    // L
                                dec_read_to_reg  = 1;
                                dec_inc_memptr   = 1;
                                dec_set_addr_to  = aZI;
                            end
                            3'd5: begin
                                dec_set_bus_a_to = 4'b0100;    // H
                                dec_read_to_reg  = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h22: begin
                        // LD (nn),HL
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_ldz          = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to  = aZI;
                                dec_inc_pc       = 1;
                                dec_ldw          = 1;
                                dec_set_bus_b_to = 4'b0101;  // L
                            end
                            3'd4: begin
                                dec_inc_memptr   = 1;
                                dec_set_addr_to  = aZI;
                                dec_write        = 1;
                                dec_set_bus_b_to = 4'b0100;  // H
                            end
                            3'd5: begin
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hf9: begin    // LD q_reg_sp,HL
                        dec_mcycles   = 3'd1;
                        dec_tstates   = 3'd6;
                        dec_is_ldsphl = 1;
                    end

                    8'hc5,8'hd5,8'he5,8'hf5: begin  // PUSH qq
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_tstates      = 3'd5;
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = (ir_dpair == 2'b11) ? 4'b0111 : {1'b0, ir_dpair, 1'b0};
                            end
                            3'd2: begin
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = (ir_dpair == 2'b11) ? 4'b1011 : {1'b0, ir_dpair, 1'b1};
                                dec_write        = 1;
                            end
                            3'd3: begin
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc1,8'hd1,8'he1,8'hf1: begin  // POP qq
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aSP;
                            end
                            3'd2: begin
                                dec_incdec16     = 4'b0111;
                                dec_set_addr_to  = aSP;
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (ir_dpair == 2'b11) ? 4'b1011 : {1'b0, ir_dpair, 1'b1};
                            end
                            3'd3: begin
                                dec_incdec16     = 4'b0111;
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (ir_dpair == 2'b11) ? 4'b0111 : {1'b0, ir_dpair, 1'b0};
                            end
                            default: begin end
                        endcase
                    end

                    // EXCHANGE, BLOCK TRANSFER AND SEARCH GROUP
                    8'heb: dec_is_ex_de_hl = 1; // EX DE,HL
                    8'h08: dec_is_ex_af    = 1; // EX AF,AF'
                    8'hd9: dec_is_exx      = 1; // EXX

                    8'he3: begin    // EX (SP),HL
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aSP;
                            end
                            3'd2: begin
                                dec_set_addr_to  = aSP;
                                dec_ldz          = 1;
                                dec_incdec16     = 4'b0111; // SP = SP+1
                            end
                            3'd3: begin
                                dec_tstates      = 3'd4;
                                dec_set_bus_b_to = 4'b0100;
                                dec_set_addr_to  = aSP;
                                dec_ldw          = 1;
                            end
                            3'd4: begin
                                dec_set_bus_b_to = 4'b0101;
                                dec_write        = 1;
                                dec_incdec16     = 4'b1111; // SP = SP-1
                                dec_set_addr_to  = aSP;
                            end
                            3'd5: begin
                                dec_exchange_wh  = 1;       // save MEMPTR to HL
                                dec_tstates      = 3'd5;
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    // 8 BIT ARITHMETIC AND LOGICAL GROUP
                    8'h80,8'h81,8'h82,8'h83,8'h84,8'h85,8'h87,          // ADD A,r
                    8'h88,8'h89,8'h8a,8'h8b,8'h8c,8'h8d,8'h8f,          // ADC A,r
                    8'h90,8'h91,8'h92,8'h93,8'h94,8'h95,8'h97,          // SUB A,r
                    8'h98,8'h99,8'h9a,8'h9b,8'h9c,8'h9d,8'h9f,          // SBC A,r
                    8'ha0,8'ha1,8'ha2,8'ha3,8'ha4,8'ha5,8'ha7,          // AND A,r
                    8'ha8,8'ha9,8'haa,8'hab,8'hac,8'had,8'haf,          // OR A,r
                    8'hb0,8'hb1,8'hb2,8'hb3,8'hb4,8'hb5,8'hb7,          // XOR A,r
                    8'hb8,8'hb9,8'hba,8'hbb,8'hbc,8'hbd,8'hbf: begin    // CP A,r
                        dec_mcycles      = 3'd1;
                        dec_set_bus_b_to = {1'b0, ir_sss};
                        dec_set_bus_a_to = 4'b0111;
                        dec_read_to_reg  = 1;
                        dec_save_alu     = 1;
                    end

                    8'h86,          // ADD A,(HL)
                    8'h8e,          // ADC A,(HL)
                    8'h96,          // SUB A,(HL)
                    8'h9e,          // SBC A,(HL)
                    8'ha6,          // AND A,(HL)
                    8'hae,          // OR A,(HL)
                    8'hb6,          // XOR A,(HL)
                    8'hbe: begin    // CP A,(HL)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                            end
                            3'd2: begin
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_set_bus_b_to = {1'b0, ir_sss};
                                dec_set_bus_a_to = 4'b0111;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc6,          // ADD A,n
                    8'hce,          // ADC A,n
                    8'hd6,          // SUB A,n
                    8'hde,          // SBC A,n
                    8'he6,          // AND A,n
                    8'hee,          // OR A,n
                    8'hf6,          // XOR A,n
                    8'hfe: begin    // CP A,n
                        dec_mcycles = 3'd2;
                        if (q_mcycle == 3'd2) begin
                            dec_inc_pc       = 1;
                            dec_read_to_reg  = 1;
                            dec_save_alu     = 1;
                            dec_set_bus_b_to = {1'b0, ir_sss};
                            dec_set_bus_a_to = 4'b0111;
                        end
                    end

                    8'h04,8'h0c,8'h14,8'h1c,8'h24,8'h2c,8'h3c: begin    // INC r
                        dec_mcycles      = 3'd1;
                        dec_set_bus_b_to = 4'b1010;
                        dec_set_bus_a_to = {1'b0, ir_ddd};
                        dec_read_to_reg  = 1;
                        dec_save_alu     = 1;
                        dec_preserve_c   = 1;
                        dec_alu_op       = AluOpAdd;
                    end

                    8'h34: begin    // INC (HL)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                            end
                            3'd2: begin
                                dec_tstates      = 3'd4;
                                dec_set_addr_to  = aXY;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_preserve_c   = 1;
                                dec_alu_op       = AluOpAdd;
                                dec_set_bus_b_to = 4'b1010;
                                dec_set_bus_a_to = {1'b0, ir_ddd};
                            end
                            3'd3: begin
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h05,8'h0d,8'h15,8'h1d,8'h25,8'h2d,8'h3d: begin    // DEC r
                        dec_mcycles      = 3'd1;
                        dec_set_bus_b_to = 4'b1010;
                        dec_set_bus_a_to = {1'b0, ir_ddd};
                        dec_read_to_reg  = 1;
                        dec_save_alu     = 1;
                        dec_preserve_c   = 1;
                        dec_alu_op       = AluOpSub;
                    end

                    8'h35: begin    // DEC (HL)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                            end
                            3'd2: begin
                                dec_tstates      = 3'd4;
                                dec_set_addr_to  = aXY;
                                dec_alu_op       = AluOpSub;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_preserve_c   = 1;
                                dec_set_bus_b_to = 4'b1010;
                                dec_set_bus_a_to = {1'b0, ir_ddd};
                            end
                            3'd3: begin
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    // GENERAL PURPOSE ARITHMETIC AND CPU CONTROL GROUPS
                    8'h27: begin    // DAA
                        dec_mcycles      = 3'd1;
                        dec_set_bus_a_to = 4'b0111;
                        dec_read_to_reg  = 1;
                        dec_alu_op       = AluOpDaa;
                        dec_save_alu     = 1;
                    end

                    8'h2f: dec_is_cpl = 1;    // CPL
                    8'h3f: dec_is_ccf = 1;    // CCF
                    8'h37: dec_is_scf = 1;    // SCF

                    8'h00: begin
                        if (q_nmi_cycle) begin
                            // NMI
                            dec_mcycles = 3'd3;
                            case (q_mcycle)
                                3'd1: begin
                                    dec_tstates      = 3'd5;
                                    dec_incdec16     = 4'b1111;
                                    dec_set_addr_to  = aSP;
                                    dec_set_bus_b_to = 4'b1101;
                                end
                                3'd2: begin
                                    dec_write        = 1;
                                    dec_incdec16     = 4'b1111;
                                    dec_set_addr_to  = aSP;
                                    dec_set_bus_b_to = 4'b1100;
                                end
                                3'd3: begin
                                    dec_write        = 1;
                                end
                                default: begin end
                            endcase

                        end else if (q_irq_cycle) begin
                            // INT (IM 2)
                            dec_mcycles = 3'd5;
                            case (q_mcycle)
                                3'd1: begin
                                    dec_tstates      = 3'd5;
                                    dec_incdec16     = 4'b1111;
                                    dec_set_addr_to  = aSP;
                                    dec_set_bus_b_to = 4'b1101;
                                end
                                3'd2: begin
                                    dec_write        = 1;
                                    dec_incdec16     = 4'b1111;
                                    dec_set_addr_to  = aSP;
                                    dec_set_bus_b_to = 4'b1100;
                                end
                                3'd3: begin
                                    dec_write        = 1;
                                end
                                3'd4: begin
                                    dec_inc_pc       = 1;
                                    dec_ldz          = 1;
                                end
                                3'd5: begin
                                    dec_jump         = 1;
                                end
                                default: begin end
                            endcase

                        end else begin
                            // NOP
                        end
                    end

                    8'h76: dec_is_halt = 1;   // HALT
                    8'hf3: dec_is_di = 1;     // DI
                    8'hfb: dec_is_ei = 1;     // EI

                    // 16 BIT ARITHMETIC GROUP
                    8'h09,8'h19,8'h29,8'h39: begin  // ADD HL,ss
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_no_pc = 1;
                            end
                            3'd2: begin
                                dec_no_read      = 1;
                                dec_alu_op       = AluOpAdd;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_set_bus_a_to = 4'b0101;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1000 : {1'b0, q_instruction[5:4], 1'b1};
                                dec_tstates      = 3'd4;
                                dec_arith16      = 1;
                                dec_set_sw       = 2'b11;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_no_read      = 1;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_alu_op       = AluOpAdc;
                                dec_set_bus_a_to = 4'b0100;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1001 : {1'b0, q_instruction[5:4], 1'b0};
                                dec_arith16      = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h03,8'h13,8'h23,8'h33: begin  // INC ss
                        dec_mcycles  = 3'd1;
                        dec_tstates  = 3'd6;
                        dec_incdec16 = {2'b01, ir_dpair};
                    end

                    8'h0b,8'h1b,8'h2b,8'h3b: begin  // DEC ss
                        dec_mcycles  = 3'd1;
                        dec_tstates  = 3'd6;
                        dec_incdec16 = {2'b11, ir_dpair};
                    end

                    // ROTATE AND SHIFT GROUP
                    8'h07,8'h17,8'h0f,8'h1f: begin  // RLCA|RLA|RRCA|RRA
                        dec_mcycles      = 3'd1;
                        dec_set_bus_a_to = 4'b0111;
                        dec_alu_op       = AluOpRot;
                        dec_read_to_reg  = 1;
                        dec_save_alu     = 1;
                    end

                    // JUMP GROUP
                    8'hc3: begin    // JP nn
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_ldz    = 1;
                            end
                            3'd3: begin
                                dec_inc_pc = 1;
                                dec_jump   = 1;
                                dec_ldw    = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc2,8'hca,8'hd2,8'hda,8'he2,8'hea,8'hf2,8'hfa: begin  // JP cc,nn
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_ldz    = 1;
                            end
                            3'd3: begin
                                dec_ldw    = 1;
                                dec_inc_pc = 1;
                                if (cc_is_true)
                                    dec_jump = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h18: begin    // JR e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_no_pc  = 1;
                            end
                            3'd3: begin
                                dec_no_read = 1;
                                dec_jump_e  = 1;
                                dec_tstates = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'h38: begin    // JR C,e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                if (!q_reg_f[Flag_C])
                                    dec_mcycles = 3'd2;
                                else
                                    dec_no_pc = 1;
                            end
                            3'd3: begin
                                dec_no_read = 1;
                                dec_jump_e  = 1;
                                dec_tstates = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'h30: begin    // JR NC,e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                if (q_reg_f[Flag_C])
                                    dec_mcycles = 3'd2;
                                else
                                    dec_no_pc = 1;
                            end
                            3'd3: begin
                                dec_no_read = 1;
                                dec_jump_e  = 1;
                                dec_tstates = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'h28: begin    // JR Z,e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                if (!q_reg_f[Flag_Z])
                                    dec_mcycles = 3'd2;
                                else
                                    dec_no_pc = 1;
                            end
                            3'd3: begin
                                dec_no_read = 1;
                                dec_jump_e  = 1;
                                dec_tstates = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'h20: begin    // JR NZ,e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                if (q_reg_f[Flag_Z])
                                    dec_mcycles = 3'd2;
                                else
                                    dec_no_pc = 1;
                            end
                            3'd3: begin
                                dec_no_read = 1;
                                dec_jump_e  = 1;
                                dec_tstates = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'he9: dec_is_jp_ind_hl = 1;  // JP (HL)

                    8'h10: begin    // DJNZ,e
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_tstates      = 3'd5;
                                dec_is_djnz      = 1;
                                dec_set_bus_b_to = 4'b1010;
                                dec_set_bus_a_to = 4'b0000;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_alu_op       = AluOpSub;
                            end
                            3'd2: begin
                                dec_is_djnz      = 1;
                                dec_inc_pc       = 1;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_no_read      = 1;
                                dec_jump_e       = 1;
                                dec_tstates      = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    // CALL AND RETURN GROUP
                    8'hcd: begin    // CALL nn
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_ldz          = 1;
                            end
                            3'd3: begin
                                dec_incdec16     = 4'b1111;
                                dec_inc_pc       = 1;
                                dec_tstates      = 3'd4;
                                dec_set_addr_to  = aSP;
                                dec_ldw          = 1;
                                dec_set_bus_b_to = 4'b1101;
                            end
                            3'd4: begin
                                dec_write        = 1;
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = 4'b1100;
                            end
                            3'd5: begin
                                dec_write        = 1;
                                dec_call         = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc4,8'hcc,8'hd4,8'hdc,8'he4,8'hec,8'hf4,8'hfc: begin  // CALL cc,nn
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc = 1;
                                dec_ldz    = 1;
                            end
                            3'd3: begin
                                dec_inc_pc = 1;
                                dec_ldw    = 1;
                                if (cc_is_true) begin
                                    dec_incdec16     = 4'b1111;
                                    dec_set_addr_to  = aSP;
                                    dec_tstates      = 3'd4;
                                    dec_set_bus_b_to = 4'b1101;
                                end else begin
                                    dec_mcycles      = 3'd3;
                                end
                            end
                            3'd4: begin
                                dec_write        = 1;
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = 4'b1100;
                            end
                            3'd5: begin
                                dec_write = 1;
                                dec_call  = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc9: begin    // RET
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to = aSP;
                            end
                            3'd2: begin
                                dec_incdec16    = 4'b0111;
                                dec_set_addr_to = aSP;
                                dec_ldz         = 1;
                            end
                            3'd3: begin
                                dec_jump        = 1;
                                dec_incdec16    = 4'b0111;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc0,8'hc8,8'hd0,8'hd8,8'he0,8'he8,8'hf0,8'hf8: begin  // RET cc
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                if (cc_is_true)
                                    dec_set_addr_to = aSP;
                                else
                                    dec_mcycles = 3'd1;

                                dec_tstates = 3'd5;
                            end
                            3'd2: begin
                                dec_incdec16    = 4'b0111;
                                dec_set_addr_to = aSP;
                                dec_ldz         = 1;
                            end
                            3'd3: begin
                                dec_jump        = 1;
                                dec_incdec16    = 4'b0111;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc7,8'hcf,8'hd7,8'hdf,8'he7,8'hef,8'hf7,8'hff: begin  // RST p
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_tstates      = 3'd5;
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = 4'b1101;
                            end
                            3'd2: begin
                                dec_write        = 1;
                                dec_incdec16     = 4'b1111;
                                dec_set_addr_to  = aSP;
                                dec_set_bus_b_to = 4'b1100;
                            end
                            3'd3: begin
                                dec_write        = 1;
                                dec_rst_p        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    // INPUT AND OUTPUT GROUP
                    8'hdb: begin    // IN A,(n)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc      = 1;
                                dec_set_addr_to = aIOA;
                            end
                            3'd3: begin
                                dec_read_to_acc = 1;
                                dec_iorq        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'hd3: begin    // OUT (n),A
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_set_addr_to  = aIOA;
                                dec_set_bus_b_to = 4'b0111;
                            end
                            3'd3: begin
                                dec_write        = 1;
                                dec_iorq         = 1;
                            end
                            default: begin end
                        endcase
                    end

                    //--------------------------------------------------------
                    // MULTIBYTE INSTRUCTIONS
                    //--------------------------------------------------------
                    8'hcb:       dec_prefix = PrefixCB;
                    8'hed:       dec_prefix = PrefixED;
                    8'hdd,8'hfd: dec_prefix = PrefixDD_FD;

                    default: begin end
                endcase
            end

            //----------------------------------------------------------------
            // CB prefixed instructions
            //----------------------------------------------------------------
            2'b01: begin
                dec_set_bus_a_to[2:0] = q_instruction[2:0];
                dec_set_bus_b_to[2:0] = q_instruction[2:0];

                case (q_instruction)
                    8'h00,8'h01,8'h02,8'h03,8'h04,8'h05,8'h07,          // RLC r
                    8'h08,8'h09,8'h0a,8'h0b,8'h0c,8'h0d,8'h0f,          // RRC r
                    8'h10,8'h11,8'h12,8'h13,8'h14,8'h15,8'h17,          // RL r
                    8'h18,8'h19,8'h1a,8'h1b,8'h1c,8'h1d,8'h1f,          // RR r
                    8'h20,8'h21,8'h22,8'h23,8'h24,8'h25,8'h27,          // SLA r
                    8'h28,8'h29,8'h2a,8'h2b,8'h2c,8'h2d,8'h2f,          // SRA r
                    8'h30,8'h31,8'h32,8'h33,8'h34,8'h35,8'h37,          // SLL r
                    8'h38,8'h39,8'h3a,8'h3b,8'h3c,8'h3d,8'h3f: begin    // SRL r
                        if (q_xy_state == 2'b00) begin
                            if (q_mcycle == 3'd1) begin
                                dec_alu_op      = AluOpRot;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                            end

                        end else begin
                            // R/S (IX+d),Reg, undocumented
                            dec_mcycles   = 3'd3;
                            dec_xybit_undoc = 1;
                            case (q_mcycle)
                                3'd1,3'd7: begin
                                    dec_set_addr_to = aXY;
                                end
                                3'd2: begin
                                    dec_alu_op      = AluOpRot;
                                    dec_read_to_reg = 1;
                                    dec_save_alu    = 1;
                                    dec_set_addr_to = aXY;
                                    dec_tstates     = 3'd4;
                                end
                                3'd3: begin
                                    dec_write       = 1;
                                end
                                default: begin end
                            endcase
                        end
                    end

                    8'h06,8'h0e,        // RLC (HL)     RRC (HL)
                    8'h16,8'h1e,        // RL (HL)      RR (HL)
                    8'h26,8'h2e,        // SLA (HL)     SRA (HL)
                    8'h36,8'h3e: begin  // SLL (HL)     SRL (HL)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1,3'd7: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_alu_op      = AluOpRot;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                                dec_set_addr_to = aXY;
                                dec_tstates     = 3'd4;
                            end
                            3'd3: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h40,8'h41,8'h42,8'h43,8'h44,8'h45,8'h47,
                    8'h48,8'h49,8'h4a,8'h4b,8'h4c,8'h4d,8'h4f,
                    8'h50,8'h51,8'h52,8'h53,8'h54,8'h55,8'h57,
                    8'h58,8'h59,8'h5a,8'h5b,8'h5c,8'h5d,8'h5f,
                    8'h60,8'h61,8'h62,8'h63,8'h64,8'h65,8'h67,
                    8'h68,8'h69,8'h6a,8'h6b,8'h6c,8'h6d,8'h6f,
                    8'h70,8'h71,8'h72,8'h73,8'h74,8'h75,8'h77,
                    8'h78,8'h79,8'h7a,8'h7b,8'h7c,8'h7d,8'h7f: begin
                        if (q_xy_state == 2'b00) begin
                            // BIT b,r
                            if (q_mcycle == 3'd1) begin
                                dec_set_bus_b_to = {1'b0, q_instruction[2:0]};
                                dec_alu_op       = AluOpBit;
                            end

                        end else begin
                            // BIT b,(IX+d), undocumented
                            dec_mcycles = 3'd2;
                            dec_xybit_undoc = 1;
                            case (q_mcycle)
                                3'd1,3'd7: begin
                                    dec_set_addr_to = aXY;
                                end
                                3'd2: begin
                                    dec_alu_op      = AluOpBit;
                                    dec_tstates     = 3'd4;
                                end
                                default: begin end
                            endcase
                        end
                    end

                    8'h46,8'h4e,8'h56,8'h5e,8'h66,8'h6e,8'h76,8'h7e: begin  // BIT b,(HL)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1,3'd7: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_alu_op      = AluOpBit;
                                dec_tstates     = 3'd4;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc0,8'hc1,8'hc2,8'hc3,8'hc4,8'hc5,8'hc7,
                    8'hc8,8'hc9,8'hca,8'hcb,8'hcc,8'hcd,8'hcf,
                    8'hd0,8'hd1,8'hd2,8'hd3,8'hd4,8'hd5,8'hd7,
                    8'hd8,8'hd9,8'hda,8'hdb,8'hdc,8'hdd,8'hdf,
                    8'he0,8'he1,8'he2,8'he3,8'he4,8'he5,8'he7,
                    8'he8,8'he9,8'hea,8'heb,8'hec,8'hed,8'hef,
                    8'hf0,8'hf1,8'hf2,8'hf3,8'hf4,8'hf5,8'hf7,
                    8'hf8,8'hf9,8'hfa,8'hfb,8'hfc,8'hfd,8'hff: begin
                        if (q_xy_state == 2'b00) begin
                            // SET b,r
                            if (q_mcycle == 3'd1) begin
                                dec_alu_op      = AluOpSet;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                            end

                        end else begin
                            // SET b,(IX+d),Reg, undocumented
                            dec_mcycles = 3'd3;
                            dec_xybit_undoc = 1;
                            case (q_mcycle)
                                3'd1,3'd7: begin
                                    dec_set_addr_to = aXY;
                                end
                                3'd2: begin
                                    dec_alu_op      = AluOpSet;
                                    dec_read_to_reg = 1;
                                    dec_save_alu    = 1;
                                    dec_set_addr_to = aXY;
                                    dec_tstates     = 3'd4;
                                end
                                3'd3: begin
                                    dec_write = 1;
                                end
                                default: begin end
                            endcase
                        end
                    end

                    8'hc6,8'hce,8'hd6,8'hde,8'he6,8'hee,8'hf6,8'hfe: begin  // SET b,(HL)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1,3'd7: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_alu_op      = AluOpSet;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                                dec_set_addr_to = aXY;
                                dec_tstates     = 3'd4;
                            end
                            3'd3: begin
                                dec_write       = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h80,8'h81,8'h82,8'h83,8'h84,8'h85,8'h87,
                    8'h88,8'h89,8'h8a,8'h8b,8'h8c,8'h8d,8'h8f,
                    8'h90,8'h91,8'h92,8'h93,8'h94,8'h95,8'h97,
                    8'h98,8'h99,8'h9a,8'h9b,8'h9c,8'h9d,8'h9f,
                    8'ha0,8'ha1,8'ha2,8'ha3,8'ha4,8'ha5,8'ha7,
                    8'ha8,8'ha9,8'haa,8'hab,8'hac,8'had,8'haf,
                    8'hb0,8'hb1,8'hb2,8'hb3,8'hb4,8'hb5,8'hb7,
                    8'hb8,8'hb9,8'hba,8'hbb,8'hbc,8'hbd,8'hbf: begin    // RES b,r
                        if (q_xy_state == 2'b00) begin
                            if (q_mcycle == 3'd1) begin
                                dec_alu_op      = AluOpRes;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                            end
                        end else begin
                            // RES b,(IX+d),Reg, undocumented
                            dec_mcycles = 3'd3;
                            dec_xybit_undoc = 1;
                            case (q_mcycle)
                                3'd1,3'd7: begin
                                    dec_set_addr_to = aXY;
                                end
                                3'd2: begin
                                    dec_alu_op      = AluOpRes;
                                    dec_read_to_reg = 1;
                                    dec_save_alu    = 1;
                                    dec_set_addr_to = aXY;
                                    dec_tstates     = 3'd4;
                                end
                                3'd3: begin
                                    dec_write       = 1;
                                end
                                default: begin end
                            endcase
                        end
                    end

                    8'h86,8'h8e,8'h96,8'h9e,8'ha6,8'hae,8'hb6,8'hbe: begin  // RES b,(HL)
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1,3'd7: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_alu_op      = AluOpRes;
                                dec_read_to_reg = 1;
                                dec_save_alu    = 1;
                                dec_set_addr_to = aXY;
                                dec_tstates     = 3'd4;
                            end
                            3'd3: begin
                                dec_write       = 1;
                            end
                            default: begin end
                        endcase
                    end

                    default: begin end
                endcase
            end

            //----------------------------------------------------------------
            // ED prefixed instructions
            //----------------------------------------------------------------
            default: begin
                case (q_instruction)
                    8'h00,8'h01,8'h02,8'h03,8'h04,8'h05,8'h06,8'h07,
                    8'h08,8'h09,8'h0a,8'h0b,8'h0c,8'h0d,8'h0e,8'h0f,
                    8'h10,8'h11,8'h12,8'h13,8'h14,8'h15,8'h16,8'h17,
                    8'h18,8'h19,8'h1a,8'h1b,8'h1c,8'h1d,8'h1e,8'h1f,
                    8'h20,8'h21,8'h22,8'h23,8'h24,8'h25,8'h26,8'h27,
                    8'h28,8'h29,8'h2a,8'h2b,8'h2c,8'h2d,8'h2e,8'h2f,
                    8'h30,8'h31,8'h32,8'h33,8'h34,8'h35,8'h36,8'h37,
                    8'h38,8'h39,8'h3a,8'h3b,8'h3c,8'h3d,8'h3e,8'h3f,
                                                              8'h77,
                                                              8'h7f,
                    8'h80,8'h81,8'h82,8'h83,8'h84,8'h85,8'h86,8'h87,
                    8'h88,8'h89,8'h8a,8'h8b,8'h8c,8'h8d,8'h8e,8'h8f,
                    8'h90,8'h91,8'h92,8'h93,8'h94,8'h95,8'h96,8'h97,
                    8'h98,8'h99,8'h9a,8'h9b,8'h9c,8'h9d,8'h9e,8'h9f,
                                            8'ha4,8'ha5,8'ha6,8'ha7,
                                            8'hac,8'had,8'hae,8'haf,
                                            8'hb4,8'hb5,8'hb6,8'hb7,
                                            8'hbc,8'hbd,8'hbe,8'hbf,
                    8'hc0,      8'hc2,      8'hc4,8'hc5,8'hc6,8'hc7,
                    8'hc8,      8'hca,8'hcb,8'hcc,8'hcd,8'hce,8'hcf,
                    8'hd0,      8'hd2,8'hd3,8'hd4,8'hd5,8'hd6,8'hd7,
                    8'hd8,      8'hda,8'hdb,8'hdc,8'hdd,8'hde,8'hdf,
                    8'he0,8'he1,8'he2,8'he3,8'he4,8'he5,8'he6,8'he7,
                    8'he8,8'he9,8'hea,8'heb,8'hec,8'hed,8'hee,8'hef,
                    8'hf0,8'hf1,8'hf2,      8'hf4,8'hf5,8'hf6,8'hf7,
                    8'hf8,8'hf9,8'hfa,8'hfb,8'hfc,8'hfd,8'hfe,8'hff: begin
                        // NOP, undocumented
                    end

                    // 8 BIT LOAD GROUP
                    8'h57: begin    // LD A,I
                        dec_mcycles    = 3'd1;
                        dec_special_ld = 3'd4;
                        dec_tstates    = 3'd5;
                    end

                    8'h5f: begin    // LD A,R
                        dec_mcycles    = 3'd1;
                        dec_special_ld = 3'd5;
                        dec_tstates    = 3'd5;
                    end

                    8'h47: begin    // LD I,A
                        dec_mcycles    = 3'd1;
                        dec_special_ld = 3'd6;
                        dec_tstates    = 3'd5;
                    end

                    8'h4f: begin    // LD R,A
                        dec_mcycles    = 3'd1;
                        dec_special_ld = 3'd7;
                        dec_tstates    = 3'd5;
                    end

                    // 16 BIT LOAD GROUP
                    8'h4b,8'h5b,8'h6b,8'h7b: begin  // LD dd,(nn)
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_ldz          = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to  = aZI;
                                dec_inc_pc       = 1;
                                dec_ldw          = 1;
                            end
                            3'd4: begin
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (q_instruction[5:4] == 2'b11) ? 4'b1000 : {1'b0, q_instruction[5:4], 1'b1};
                                dec_inc_memptr   = 1;
                                dec_set_addr_to  = aZI;
                            end
                            3'd5: begin
                                dec_read_to_reg  = 1;
                                dec_set_bus_a_to = (q_instruction[5:4] == 2'b11) ? 4'b1001 : {1'b0, q_instruction[5:4], 1'b0};
                            end
                            default: begin end
                        endcase
                    end

                    8'h43,8'h53,8'h63,8'h73: begin  // LD (nn),dd
                        dec_mcycles = 3'd5;
                        case (q_mcycle)
                            3'd2: begin
                                dec_inc_pc       = 1;
                                dec_ldz          = 1;
                            end
                            3'd3: begin
                                dec_set_addr_to  = aZI;
                                dec_inc_pc       = 1;
                                dec_ldw          = 1;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1000 : {1'b0, q_instruction[5:4], 1'b1};
                            end
                            3'd4: begin
                                dec_inc_memptr   = 1;
                                dec_set_addr_to  = aZI;
                                dec_write        = 1;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1001 : {1'b0, q_instruction[5:4], 1'b0};
                            end
                            3'd5: begin
                                dec_write        = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'ha0,8'ha8,8'hb0,8'hb8: begin  // LDI, LDD, LDIR, LDDR
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                                dec_incdec16     = 4'b1100;      // BC
                            end
                            3'd2: begin
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_bus_a_to = 4'b0111;
                                dec_alu_op       = AluOpAdd;
                                dec_set_addr_to  = aDE;
                                dec_incdec16     = q_instruction[3] ? 4'b1110 : 4'b0110;
                            end
                            3'd3: begin
                                dec_is_bt        = 1;
                                dec_tstates      = 3'd5;
                                dec_write        = 1;
                                dec_incdec16     = q_instruction[3] ? 4'b1101 : 4'b0101;
                                dec_no_pc        = 1;
                            end
                            3'd4: begin
                                dec_no_read      = 1;
                                dec_tstates      = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'ha1,8'ha9,8'hb1,8'hb9: begin  // CPI, CPD, CPIR, CPDR
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                                dec_incdec16     = 4'b1100;  // BC
                            end
                            3'd2: begin
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_bus_a_to = 4'b0111;
                                dec_alu_op       = AluOpCp;
                                dec_save_alu     = 1;
                                dec_preserve_c   = 1;
                                dec_incdec16     = q_instruction[3] ? 4'b1110 : 4'b0110;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_no_read      = 1;
                                dec_is_bc        = 1;
                                dec_tstates      = 3'd5;
                                dec_no_pc        = 1;
                            end
                            3'd4: begin
                                dec_no_read      = 1;
                                dec_tstates      = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'h44,8'h4c,8'h54,8'h5c,8'h64,8'h6c,8'h74,8'h7c: begin  // NEG
                        dec_mcycles      = 3'd1;
                        dec_alu_op       = AluOpSub;
                        dec_set_bus_b_to = 4'b0111;
                        dec_set_bus_a_to = 4'b1010;
                        dec_read_to_acc  = 1;
                        dec_save_alu     = 1;
                    end

                    8'h46,8'h4e,8'h66,8'h6e: dec_im = 2'd0;  // IM 0
                    8'h56,8'h76:             dec_im = 2'd1;  // IM 1
                    8'h5e,8'h7e:             dec_im = 2'd2;  // IM 2

                    // 16 bit arithmetic
                    8'h4a,8'h5a,8'h6a,8'h7a: begin  // ADC HL,ss
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_no_pc        = 1;
                            end
                            3'd2: begin
                                dec_no_read      = 1;
                                dec_alu_op       = AluOpAdc;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_set_bus_a_to = 4'b0101;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1000 : {1'b0, q_instruction[5:4], 1'b1};
                                dec_tstates      = 3'd4;
                                dec_set_sw       = 2'b11;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_no_read      = 1;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_alu_op       = AluOpAdc;
                                dec_set_bus_a_to = 4'b0100;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1001 : {1'b0, q_instruction[5:4], 1'b0};
                            end
                            default: begin end
                        endcase
                    end

                    8'h42,8'h52,8'h62,8'h72: begin  // SBC HL,ss
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_no_pc        = 1;
                            end
                            3'd2: begin
                                dec_no_read      = 1;
                                dec_alu_op       = AluOpSbc;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_set_bus_a_to = 4'b0101;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1000 : {1'b0, q_instruction[5:4], 1'b1};
                                dec_tstates      = 3'd4;
                                dec_set_sw       = 2'b11;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_no_read      = 1;
                                dec_alu_op       = AluOpSbc;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_set_bus_a_to = 4'b0100;
                                dec_set_bus_b_to = (q_instruction[5:4] == 2'b11) ? 4'b1001 : {1'b0, q_instruction[5:4], 1'b0};
                            end
                            default: begin end
                        endcase
                    end

                    8'h6f: begin    // RLD -- Read in M2, not M3! fixed by Sorgelig
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aXY;
                            end
                            3'd2: begin
                                dec_read_to_reg  = 1;
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_bus_a_to = 4'b0111;
                                dec_alu_op       = AluOpRld;
                                dec_save_alu     = 1;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_tstates      = 3'd4;
                                dec_is_rld       = 1;
                                dec_no_read      = 1;
                                dec_set_addr_to  = aXY;
                            end
                            3'd4: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h67: begin    // RRD -- Read in M2, not M3! fixed by Sorgelig
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to = aXY;
                            end
                            3'd2: begin
                                dec_read_to_reg  = 1;
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_bus_a_to = 4'b0111;
                                dec_alu_op       = AluOpRrd;
                                dec_save_alu     = 1;
                                dec_no_pc        = 1;
                            end
                            3'd3: begin
                                dec_tstates      = 3'd4;
                                dec_is_rrd       = 1;
                                dec_no_read      = 1;
                                dec_set_addr_to  = aXY;
                            end
                            3'd4: begin
                                dec_write = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h45,8'h4d,8'h55,8'h5d,8'h65,8'h6d,8'h75,8'h7d: begin  // RETI/RETN
                        dec_mcycles = 3'd3;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to = aSP;
                            end
                            3'd2: begin
                                dec_incdec16    = 4'b0111;
                                dec_set_addr_to = aSP;
                                dec_ldz         = 1;
                            end
                            3'd3: begin
                                dec_jump        = 1;
                                dec_incdec16    = 4'b0111;
                                dec_ldw         = 1;
                                dec_is_retn     = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h40,8'h48,8'h50,8'h58,8'h60,8'h68,8'h70,8'h78: begin
                        // IN r,(C)
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to = aBC;
                                dec_set_sw      = 2'b01;
                            end
                            3'd2: begin
                                dec_iorq = 1;
                                if (q_instruction[5:3] != 3'd6) begin
                                    dec_read_to_reg  = 1;
                                    dec_set_bus_a_to = {1'b0, q_instruction[5:3]};
                                end
                                dec_is_inrc = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'h41,8'h49,8'h51,8'h59,8'h61,8'h69,8'h71,8'h79: begin  // OUT (C),r   OUT (C),0
                        dec_mcycles = 3'd2;
                        case (q_mcycle)
                            3'd1: begin
                                dec_set_addr_to  = aBC;
                                dec_set_sw       = 2'b01;
                                dec_set_bus_b_to = (q_instruction[5:3] == 3'd6) ? {1'b1, q_instruction[5:3]} : {1'b0, q_instruction[5:3]};
                            end
                            3'd2: begin
                                dec_write        = 1;
                                dec_iorq         = 1;
                            end
                            default: begin end
                        endcase
                    end

                    8'ha2,8'haa,8'hb2,8'hba: begin  // INI, IND, INIR, INDR
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_tstates      = 3'd5;
                                dec_set_addr_to  = aBC;
                                dec_set_bus_b_to = 4'b1010;
                                dec_set_bus_a_to = 4'b0000;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_alu_op       = AluOpSub;
                                dec_set_sw       = 2'b11;
                                dec_incdec16     = q_instruction[3] ? 4'b1000 : 4'b0000;
                            end
                            3'd2: begin
                                dec_iorq         = 1;
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_addr_to  = aXY;
                            end
                            3'd3: begin
                                dec_incdec16     = q_instruction[3] ? 4'b1110 : 4'b0110;
                                dec_write        = 1;
                                dec_is_btr       = 1;
                            end
                            3'd4: begin
                                dec_no_read      = 1;
                                dec_tstates      = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'ha3,8'hab,8'hb3,8'hbb: begin  // OUTI, OUTD, OTIR, OTDR
                        dec_mcycles = 3'd4;
                        case (q_mcycle)
                            3'd1: begin
                                dec_tstates      = 3'd5;
                                dec_set_addr_to  = aXY;
                                dec_set_bus_b_to = 4'b1010;
                                dec_set_bus_a_to = 4'b0000;
                                dec_read_to_reg  = 1;
                                dec_save_alu     = 1;
                                dec_alu_op       = AluOpSub;
                            end
                            3'd2: begin
                                dec_set_bus_b_to = 4'b0110;
                                dec_set_addr_to  = aBC;
                                dec_set_sw       = 2'b11;
                                dec_incdec16     = q_instruction[3] ? 4'b1000 : 4'b0000;
                            end
                            3'd3: begin
                                dec_incdec16     = q_instruction[3] ? 4'b1110 : 4'b0110;
                                dec_iorq         = 1;
                                dec_write        = 1;
                                dec_is_btr       = 1;
                            end
                            3'd4: begin
                                dec_no_read      = 1;
                                dec_tstates      = 3'd5;
                            end
                            default: begin end
                        endcase
                    end

                    8'hc1,8'hc9,8'hd1,8'hd9: begin end
                    8'hc3,8'hf3: begin end
                    default: begin end
                endcase
            end
        endcase

        if (q_mcycle == 3'd6) begin
            dec_inc_pc = 1;
            if (q_instruction == 8'h36 || q_instruction == 8'hcb)
                dec_set_addr_to = aNone;

            if (!(q_instruction == 8'h36 || q_prefix == PrefixCB))
                dec_no_pc = 1;
        end

        if (q_mcycle == 3'd7) begin
            dec_tstates = 3'd5;

            if (q_prefix != PrefixCB)
                dec_set_addr_to = aXY;

            dec_set_bus_b_to = {1'b0, ir_sss};

            if (q_instruction == 8'h36 || q_prefix == PrefixCB) begin
                // LD (HL),n
                dec_inc_pc = 1;
            end else begin
                dec_no_read = 1;
            end
        end
    end

    //------------------------------------------------------------------------
    // ALU
    //------------------------------------------------------------------------
    reg [7:0] alu_bitmask;
    always @* case (q_instruction[5:3])
        3'd0:    alu_bitmask = 8'h01;
        3'd1:    alu_bitmask = 8'h02;
        3'd2:    alu_bitmask = 8'h04;
        3'd3:    alu_bitmask = 8'h08;
        3'd4:    alu_bitmask = 8'h10;
        3'd5:    alu_bitmask = 8'h20;
        3'd6:    alu_bitmask = 8'h40;
        default: alu_bitmask = 8'h80;
    endcase

    wire       alu_do_sub        = (q_alu_op == AluOpSub || q_alu_op == AluOpSbc || q_alu_op == AluOpCp);
    wire       alu_cin           = (alu_do_sub ^ ((q_alu_op == AluOpAdc || q_alu_op == AluOpSbc) & q_reg_f[Flag_C]));
    wire [5:0] alu_addsub_l      = {1'b0, bus_a[3:0], alu_cin}         + {1'b0, (alu_do_sub ? ~bus_b[3:0] : bus_b[3:0]), 1'b1};
    wire [4:0] addsub_m          = {1'b0, bus_a[6:4], alu_addsub_l[5]} + {1'b0, (alu_do_sub ? ~bus_b[6:4] : bus_b[6:4]), 1'b1};
    wire [2:0] addsub_h          = {1'b0, bus_a[7],   addsub_m[4]}     + {1'b0, (alu_do_sub ? ~bus_b[7]   : bus_b[7]),   1'b1};
    wire       alu_half_carry    = alu_addsub_l[5];
    wire       alu_carry7        = addsub_m[4];
    wire       alu_carry         = addsub_h[2];
    wire [7:0] alu_addsub_result = {addsub_h[1], addsub_m[3:1], alu_addsub_l[4:1]};
    wire       alu_overflow      = alu_carry ^ alu_carry7;
    reg  [7:0] alu_result;
    reg  [8:0] alu_daa_tmp;
    reg  [7:0] d_reg_f;

    always @* begin
        alu_result  = 0;
        d_reg_f     = q_reg_f;
        alu_daa_tmp = 0;

        case (q_alu_op)
            AluOpAdd, AluOpAdc, AluOpSub, AluOpSbc, AluOpAnd, AluOpXor, AluOpOr, AluOpCp: begin
                d_reg_f[Flag_N] = 0;
                d_reg_f[Flag_C] = 0;

                case (q_alu_op)
                    AluOpAdd, AluOpAdc: begin
                        alu_result      = alu_addsub_result;
                        d_reg_f[Flag_C] = alu_carry;
                        d_reg_f[Flag_H] = alu_half_carry;
                        d_reg_f[Flag_P] = alu_overflow;
                    end

                    AluOpSub, AluOpSbc, AluOpCp: begin
                        alu_result      = alu_addsub_result;
                        d_reg_f[Flag_N] = 1;
                        d_reg_f[Flag_C] = !alu_carry;
                        d_reg_f[Flag_H] = !alu_half_carry;
                        d_reg_f[Flag_P] = alu_overflow;
                    end

                    AluOpAnd: begin alu_result = bus_a & bus_b; d_reg_f[Flag_H] = 1; end
                    AluOpXor: begin alu_result = bus_a ^ bus_b; d_reg_f[Flag_H] = 0; end
                    AluOpOr:  begin alu_result = bus_a | bus_b; d_reg_f[Flag_H] = 0; end

                    default: begin end
                endcase

                d_reg_f[Flag_X] = (q_alu_op == AluOpCp) ? bus_b[3] : alu_result[3];
                d_reg_f[Flag_Y] = (q_alu_op == AluOpCp) ? bus_b[5] : alu_result[5];
                d_reg_f[Flag_Z] = q_z16 ? q_reg_f[Flag_Z] : (alu_result == 8'b0);
                d_reg_f[Flag_S] = alu_result[7];

                case (q_alu_op)
                    AluOpAdd, AluOpAdc, AluOpSub, AluOpSbc, AluOpCp: begin end
                    default: begin
                        d_reg_f[Flag_P] = !(alu_result[0] ^ alu_result[1] ^ alu_result[2] ^ alu_result[3] ^ alu_result[4] ^ alu_result[5] ^ alu_result[6] ^ alu_result[7]);
                    end
                endcase

                if (q_arith16) begin
                    d_reg_f[Flag_S] = q_reg_f[Flag_S];
                    d_reg_f[Flag_Z] = q_reg_f[Flag_Z];
                    d_reg_f[Flag_P] = q_reg_f[Flag_P];
                end
            end

            AluOpDaa: begin
                d_reg_f[Flag_H] = q_reg_f[Flag_H];
                d_reg_f[Flag_C] = q_reg_f[Flag_C];
                alu_daa_tmp     = {1'b0, bus_a};

                if (!q_reg_f[Flag_N]) begin
                    // After addition
                    // A_low > 9 or H = 1
                    if (alu_daa_tmp[3:0] > 4'd9 || q_reg_f[Flag_H]) begin
                        d_reg_f[Flag_H] = (alu_daa_tmp[3:0] > 4'd9);
                        alu_daa_tmp     = alu_daa_tmp + 9'd6;
                    end

                    // new A_high > 9 or C = 1
                    if (alu_daa_tmp[8:4] > 5'd9 || q_reg_f[Flag_C])
                        alu_daa_tmp = alu_daa_tmp + 9'h60;

                end else begin
                    // After subtraction
                    if (alu_daa_tmp[3:0] > 4'd9 || q_reg_f[Flag_H]) begin
                        if (alu_daa_tmp[3:0] > 4'd5)
                            d_reg_f[Flag_H] = 0;

                        alu_daa_tmp[7:0] = alu_daa_tmp[7:0] - 8'd6;
                    end

                    if (bus_a > 8'd153 || q_reg_f[Flag_C])
                        alu_daa_tmp = alu_daa_tmp - 9'h160;
                end

                alu_result      = alu_daa_tmp[7:0];
                d_reg_f[Flag_X] = alu_daa_tmp[3];
                d_reg_f[Flag_Y] = alu_daa_tmp[5];
                d_reg_f[Flag_C] = q_reg_f[Flag_C] | alu_daa_tmp[8];
                d_reg_f[Flag_Z] = (alu_daa_tmp[7:0] == 8'b0);
                d_reg_f[Flag_S] = alu_daa_tmp[7];
                d_reg_f[Flag_P] = !(alu_daa_tmp[0] ^ alu_daa_tmp[1] ^ alu_daa_tmp[2] ^ alu_daa_tmp[3] ^ alu_daa_tmp[4] ^ alu_daa_tmp[5] ^ alu_daa_tmp[6] ^ alu_daa_tmp[7]);
            end

            AluOpRld, AluOpRrd: begin
                alu_result      = {bus_a[7:4], (q_alu_op == AluOpRld) ? bus_b[7:4] : bus_b[3:0]};
                d_reg_f[Flag_H] = 0;
                d_reg_f[Flag_N] = 0;
                d_reg_f[Flag_X] = alu_result[3];
                d_reg_f[Flag_Y] = alu_result[5];
                d_reg_f[Flag_Z] = (alu_result == 8'b0);
                d_reg_f[Flag_S] = alu_result[7];
                d_reg_f[Flag_P] = !(alu_result[0] ^ alu_result[1] ^ alu_result[2] ^ alu_result[3] ^ alu_result[4] ^ alu_result[5] ^ alu_result[6] ^ alu_result[7]);
            end

            AluOpBit: begin
                alu_result      = bus_b & alu_bitmask;
                d_reg_f[Flag_S] = alu_result[7];
                d_reg_f[Flag_Z] = (alu_result == 8'b0);
                d_reg_f[Flag_P] = (alu_result == 8'b0);
                d_reg_f[Flag_H] = 1;
                d_reg_f[Flag_N] = 0;
                if (q_instruction[2:0] == 3'd6 || q_xy_state != 2'b00) begin
                    d_reg_f[Flag_X] = q_memptr[11];
                    d_reg_f[Flag_Y] = q_memptr[13];
                end else begin
                    d_reg_f[Flag_X] = bus_b[3];
                    d_reg_f[Flag_Y] = bus_b[5];
                end
            end

            AluOpSet: begin
                alu_result = bus_b | alu_bitmask;
            end

            AluOpRes: begin
                alu_result = bus_b & ~alu_bitmask;
            end

            AluOpRot: begin
                case (q_instruction[5:3])
                    3'd0:    begin alu_result = {bus_a[6:0],      bus_a[7]};        d_reg_f[Flag_C] = bus_a[7]; end // RLC
                    3'd2:    begin alu_result = {bus_a[6:0],      q_reg_f[Flag_C]}; d_reg_f[Flag_C] = bus_a[7]; end // RL
                    3'd1:    begin alu_result = {bus_a[0],        bus_a[7:1]};      d_reg_f[Flag_C] = bus_a[0]; end // RRC
                    3'd3:    begin alu_result = {q_reg_f[Flag_C], bus_a[7:1]};      d_reg_f[Flag_C] = bus_a[0]; end // RR
                    3'd4:    begin alu_result = {bus_a[6:0],      1'b0};            d_reg_f[Flag_C] = bus_a[7]; end // SLA
                    3'd6:    begin alu_result = {bus_a[6:0],      1'b1};            d_reg_f[Flag_C] = bus_a[7]; end // SLL (Undocumented) / SWAP
                    3'd5:    begin alu_result = {bus_a[7],        bus_a[7:1]};      d_reg_f[Flag_C] = bus_a[0]; end // SRA
                    default: begin alu_result = {1'b0,            bus_a[7:1]};      d_reg_f[Flag_C] = bus_a[0]; end // SRL
                endcase

                d_reg_f[Flag_H] = 0;
                d_reg_f[Flag_N] = 0;
                d_reg_f[Flag_X] = alu_result[3];
                d_reg_f[Flag_Y] = alu_result[5];
                d_reg_f[Flag_S] = alu_result[7];
                d_reg_f[Flag_Z] = (alu_result == 8'b0);
                d_reg_f[Flag_P] = !(alu_result[0] ^ alu_result[1] ^ alu_result[2] ^ alu_result[3] ^ alu_result[4] ^ alu_result[5] ^ alu_result[6] ^ alu_result[7]);

                if (q_prefix == PrefixNone) begin
                    d_reg_f[Flag_P] = q_reg_f[Flag_P];
                    d_reg_f[Flag_S] = q_reg_f[Flag_S];
                    d_reg_f[Flag_Z] = q_reg_f[Flag_Z];
                end
            end

            default: begin end
        endcase
    end

    //------------------------------------------------------------------------

    wire       really_wait      = t80_wait & (dec_write | ~dec_no_read);
    wire       t_reset          = q_tstate == dec_tstates;
    wire       next_is_xy_fetch = q_xy_state != 2'b00 && !q_xy_ind && (dec_set_addr_to == aXY || (q_mcycle == 3'd1 && q_instruction == 8'hcb) || (q_mcycle == 3'd1 && q_instruction == 8'h36));
    wire [7:0] save_mux         = dec_exchange_rp ? bus_b : !q_save_alu ? q_di : alu_result;

    reg        q_is_rld_rrd;

    wire [8:0] ioq1   = {1'b0, q_di} + {1'b0, incdec16_result[7:0]};
    wire [8:0] ioq2   = (ioq1 & 9'b000000111) ^ {1'b0, bus_a};
    wire [7:0] temp_n = alu_result - {7'b0, d_reg_f[Flag_H]};

    always @(posedge clk) begin
        if (reset) begin
            q_reg_pc           <= 0;
            q_reg_a            <= 8'hFF;
            q_reg_f            <= 8'hFF;
            q_reg_a_alt        <= 8'hFF;
            q_reg_f_alt        <= 8'hFF;
            q_reg_i            <= 0;
            q_reg_r            <= 0;
            q_reg_sp           <= 16'hFFFF;

            q_bus_addr         <= 0;
            q_bus_wrdata       <= 0;

            q_memptr           <= 0;
            q_instruction      <= 8'h00;
            q_prefix           <= 0;
            q_xy_state         <= 0;

            q_im               <= 0;
            q_mcycles          <= 0;
            q_regs_alt         <= 0;
            q_read_to_reg      <= 0;
            q_arith16          <= 0;
            q_btr              <= 0;
            q_z16              <= 0;
            q_alu_op           <= AluOpAdd;
            q_save_alu         <= 0;
            q_preserve_c       <= 0;
            q_xy_ind           <= 0;
            q_is_rld_rrd       <= 0;

        end else begin
            if (clk_en) begin
                q_alu_op      <= AluOpAdd;
                q_save_alu    <= 0;
                q_read_to_reg <= 5'b00000;
                q_mcycles     <= dec_mcycles;
                q_im          <= dec_im;
                q_arith16     <= dec_arith16;
                q_preserve_c  <= dec_preserve_c;
                q_z16         <= (q_prefix == PrefixED && !dec_alu_op[2] && dec_alu_op[0] && q_mcycle == 3'd3);

                if (q_mcycle == 3'd1 && !q_tstate[2]) begin
                    if (q_tstate == 2 && !t80_wait) begin
                        q_bus_addr <= {q_reg_i, q_reg_r};
                        q_reg_r[6:0]  <= q_reg_r[6:0] + 7'd1;
                        if (!dec_jump && !dec_call && !q_nmi_cycle && !q_irq_cycle && !(q_halt || dec_is_halt)) begin
                            q_reg_pc <= q_reg_pc + 1;
                        end

                        if (q_irq_cycle && q_im == 2'b01)
                            q_instruction <= 8'hFF;
                        else if (q_halt || (q_irq_cycle && q_im == 2'b10) || q_nmi_cycle)
                            q_instruction <= 8'h00;
                        else
                            q_instruction <= d_rddata;
                        
                        if (q_irq_cycle && q_im == 2'b10)   // IM2 vector address low byte from bus
                            q_memptr[7:0] <= irq_vector;

                        q_prefix <= PrefixNone;
                        if (dec_prefix != PrefixNone) begin
                            if (dec_prefix == PrefixDD_FD) begin
                                q_xy_state <= q_instruction[5] ? 2'b10 : 2'b01;
                            end else begin
                                if (dec_prefix == PrefixED) begin
                                    q_xy_state <= 2'b00;
                                    q_xy_ind   <= 0;
                                end
                                q_prefix <= dec_prefix;
                            end
                        end else begin
                            q_xy_state <= 2'b00;
                            q_xy_ind   <= 0;
                        end
                    end

                end else begin
                    if (q_mcycle == 3'd6) begin
                        q_xy_ind <= 1;
                        if (dec_prefix == PrefixCB)
                            q_prefix <= dec_prefix;
                    end

                    if (t_reset) begin
                        q_btr <= (dec_is_bt | dec_is_bc | dec_is_btr) & ~q_no_btr;
                        if (dec_jump) begin
                            q_bus_addr <= {q_di, q_memptr[7:0]};
                            q_reg_pc   <= {q_di, q_memptr[7:0]};

                        end else if (dec_is_jp_ind_hl) begin
                            q_bus_addr <= reg_bus_c;
                            q_reg_pc   <= reg_bus_c;

                        end else if (dec_call || dec_rst_p) begin
                            q_bus_addr <= q_memptr;
                            q_reg_pc   <= q_memptr;

                        end else if (q_mcycle == q_mcycles && q_nmi_cycle) begin
                            q_bus_addr <= 16'h0066;
                            q_reg_pc   <= 16'h0066;

                        end else if (q_mcycle == 3'd3 && q_irq_cycle && q_im == 2'b10) begin
                            q_bus_addr <= {q_reg_i, q_memptr[7:0]};
                            q_reg_pc   <= {q_reg_i, q_memptr[7:0]};

                        end else begin
                            case (dec_set_addr_to)
                                aXY: begin
                                    if (q_xy_state == 2'b00)
                                        q_bus_addr <= reg_bus_c;
                                    else
                                        q_bus_addr <= next_is_xy_fetch ? q_reg_pc : q_memptr;
                                end
                                aIOA: begin
                                    q_bus_addr <= {q_reg_a, q_di};
                                    q_memptr   <= {q_reg_a, q_di} + 16'd1;
                                end
                                aSP: begin
                                    q_bus_addr <= q_reg_sp;
                                end
                                aBC: begin
                                    q_bus_addr <= reg_bus_c;
                                    if (dec_set_sw == 2'b01) begin
                                        q_memptr <= reg_bus_c + 1'b1;
                                    end
                                    if (dec_set_sw == 2'b10) begin
                                        q_memptr[15:8] <= q_reg_a;
                                        q_memptr[7:0]  <= reg_bus_c[7:0] + 1'b1;
                                    end
                                end
                                aDE: begin
                                    q_bus_addr <= reg_bus_c;
                                    if (dec_set_sw == 2'b10) begin
                                        q_memptr[15:8] <= q_reg_a;
                                        q_memptr[7:0]  <= reg_bus_c[7:0] + 1'b1;
                                    end
                                end
                                aZI: begin
                                    if (dec_inc_memptr) begin
                                        q_bus_addr <= q_memptr + 16'd1;
                                    end else begin
                                        q_bus_addr <= {q_di, q_memptr[7:0]};
                                        if (dec_set_sw == 2'b10) begin
                                            q_memptr[15:8] <= q_reg_a;
                                            q_memptr[7:0]  <= q_memptr[7:0] + 1'b1;
                                        end
                                    end
                                end
                                default: begin
                                    if (q_prefix == PrefixED && q_instruction[7:4] == 4'hB && q_instruction[2:1] == 2'b01 && q_mcycle == 3 && !q_no_btr) begin
                                        // INIR, INDR, OTIR, OTDR
                                        q_bus_addr <= q_reg_bus_a;
                                    end
                                    else if (!dec_no_pc || q_no_btr || (dec_is_djnz && q_inc_dec_is_zero)) begin
                                        q_bus_addr <= q_reg_pc;
                                    end
                                end
                            endcase
                        end
                        if (dec_set_sw == 2'b11) begin
                            q_memptr <= incdec16_result;
                        end

                        q_save_alu <= dec_save_alu;
                        q_alu_op   <= dec_alu_op;

                        if (dec_is_cpl) begin     // CPL
                            q_reg_a         <= ~q_reg_a;
                            q_reg_f[Flag_Y] <= ~q_reg_a[5];
                            q_reg_f[Flag_H] <= 1;
                            q_reg_f[Flag_X] <= ~q_reg_a[3];
                            q_reg_f[Flag_N] <= 1;
                        end

                        if (dec_is_ccf) begin     // CCF
                            q_reg_f[Flag_C] <= ~q_reg_f[Flag_C];
                            q_reg_f[Flag_Y] <= q_reg_a[5];
                            q_reg_f[Flag_H] <= q_reg_f[Flag_C];
                            q_reg_f[Flag_X] <= q_reg_a[3];
                            q_reg_f[Flag_N] <= 0;
                        end

                        if (dec_is_scf) begin     // SCF
                            q_reg_f[Flag_C] <= 1;
                            q_reg_f[Flag_Y] <= q_reg_a[5];
                            q_reg_f[Flag_H] <= 0;
                            q_reg_f[Flag_X] <= q_reg_a[3];
                            q_reg_f[Flag_N] <= 0;
                        end
                    end

                    if ((q_tstate == 2 && !really_wait && dec_is_btr && q_instruction[0]) || (q_tstate == 1 && dec_is_btr && !q_instruction[0])) begin
                        q_reg_f[Flag_N] <= q_di[7];
                        q_reg_f[Flag_C] <= ioq1[8];
                        q_reg_f[Flag_H] <= ioq1[8];
                        q_reg_f[Flag_P] <= ~(ioq2[0] ^ ioq2[1] ^ ioq2[2] ^ ioq2[3] ^ ioq2[4] ^ ioq2[5] ^ ioq2[6] ^ ioq2[7]);
                    end

                    if (q_tstate == 2 && !really_wait) begin
                        if (q_prefix == PrefixCB && q_mcycle == 3'd7)
                            q_instruction <= d_rddata;

                        if (dec_jump_e) begin
                            q_reg_pc <= q_reg_pc + {{8{q_di[7]}}, q_di};
                            q_memptr <= q_reg_pc + {{8{q_di[7]}}, q_di};
                        end else if (dec_inc_pc) begin
                            q_reg_pc <= q_reg_pc + 16'd1;
                        end

                        if (q_btr)
                            q_reg_pc <= q_reg_pc - 16'd2;

                        if (dec_rst_p)
                            q_memptr <= {10'b0, q_instruction[5:3], 3'b0};
                    end

                    if (q_tstate == 3 && q_mcycle == 3'd6)
                        q_memptr <= reg_bus_c + {{8{q_di[7]}}, q_di};

                    if ((dec_is_bt || dec_is_bc) && q_mcycle == 3'd3 && q_tstate == 4 && !q_no_btr)
                        q_memptr <= q_reg_pc - 16'd1;

                    if (dec_incdec16[2:0] == 3'd7 && ((q_tstate == 2 && !really_wait) || (q_tstate == 4 && q_mcycle == 3'd1)))
                        q_reg_sp <= dec_incdec16[3] ? (q_reg_sp - 16'd1) : (q_reg_sp + 16'd1);

                    if (dec_is_ldsphl)
                        q_reg_sp <= reg_bus_c;

                    if (dec_is_ex_af) begin
                        q_reg_a_alt <= q_reg_a;
                        q_reg_a     <= q_reg_a_alt;
                        q_reg_f_alt <= q_reg_f;
                        q_reg_f     <= q_reg_f_alt;
                    end

                    if (dec_is_exx)
                        q_regs_alt <= !q_regs_alt;
                end

                if (q_tstate == 3) begin
                    if (dec_ldz) q_memptr[7:0]  <= q_di;
                    if (dec_ldw) q_memptr[15:8] <= q_di;

                    if (dec_special_ld[2]) begin
                        case (dec_special_ld[1:0])
                            2'b00: begin
                                q_reg_a         <= q_reg_i;
                                q_reg_f[Flag_P] <= q_int_en2;
                                q_reg_f[Flag_S] <= q_reg_i[7];
                                q_reg_f[Flag_Z] <= (q_reg_i == 8'h00);
                                q_reg_f[Flag_Y] <= q_reg_i[5];
                                q_reg_f[Flag_H] <= 0;
                                q_reg_f[Flag_X] <= q_reg_i[3];
                                q_reg_f[Flag_N] <= 0;
                            end

                            2'b01: begin
                                q_reg_a <= q_reg_r;
                                q_reg_f[Flag_P] <= q_int_en2;
                                q_reg_f[Flag_S] <= q_reg_r[7];
                                q_reg_f[Flag_Z] <= (q_reg_r == 8'h00);
                                q_reg_f[Flag_Y] <= q_reg_r[5];
                                q_reg_f[Flag_H] <= 0;
                                q_reg_f[Flag_X] <= q_reg_r[3];
                                q_reg_f[Flag_N] <= 0;
                            end

                            2'b10:   q_reg_i <= q_reg_a;
                            default: q_reg_r <= q_reg_a;
                        endcase
                    end
                end

                if ((!dec_is_djnz && q_save_alu) || q_alu_op == AluOpBit) begin
                    q_reg_f[7:1] <= d_reg_f[7:1];

                    if (!q_preserve_c)
                        q_reg_f[Flag_C] <= d_reg_f[0];
                end

                if (t_reset && dec_is_inrc) begin
                    q_reg_f[Flag_H] <= 0;
                    q_reg_f[Flag_N] <= 0;
                    q_reg_f[Flag_X] <= q_di[3];
                    q_reg_f[Flag_Y] <= q_di[5];
                    q_reg_f[Flag_Z] <= (q_di[7:0] == 8'h00);
                    q_reg_f[Flag_S] <= q_di[7];
                    q_reg_f[Flag_P] <= ~(q_di[0] ^ q_di[1] ^ q_di[2] ^ q_di[3] ^ q_di[4] ^ q_di[5] ^ q_di[6] ^ q_di[7]);
                end

                if (q_tstate == 1 && !q_auto_wait) begin
                    // Keep D0 from M3 for RLD/RRD (Sorgelig)
                    q_is_rld_rrd <= dec_is_rld | dec_is_rrd;

                    if (!q_is_rld_rrd) q_bus_wrdata <= bus_b;
                    if (dec_is_rld)    q_bus_wrdata <= {bus_b[3:0], bus_a[3:0]};
                    if (dec_is_rrd)    q_bus_wrdata <= {bus_a[3:0], bus_b[7:4]};
                end

                if (t_reset) begin
                    q_read_to_reg <= {dec_read_to_reg, dec_set_bus_a_to};
                    if (dec_read_to_acc)
                        q_read_to_reg <= 5'b10111;
                end

                if (q_tstate == 1 && dec_is_bt) begin
                    q_reg_f[Flag_X] <= alu_result[3];
                    q_reg_f[Flag_Y] <= alu_result[1];
                    q_reg_f[Flag_H] <= 0;
                    q_reg_f[Flag_N] <= 0;
                end

                if (q_tstate == 1 && dec_is_bc) begin
                    q_reg_f[Flag_X] <= temp_n[3];
                    q_reg_f[Flag_Y] <= temp_n[1];
                end

                if (dec_is_bc || dec_is_bt)
                    q_reg_f[Flag_P] <= q_inc_dec_is_zero;

                if ((q_tstate == 1 && !q_save_alu && !q_auto_wait) || (q_save_alu && q_alu_op != AluOpCp)) begin
                    case (q_read_to_reg)
                        5'b10111: q_reg_a        <= save_mux;
                        5'b10110: q_bus_wrdata   <= save_mux;
                        5'b11000: q_reg_sp[7:0]  <= save_mux;
                        5'b11001: q_reg_sp[15:8] <= save_mux;
                        5'b11011: q_reg_f        <= save_mux;
                        default: begin end
                    endcase

                    if (dec_xybit_undoc)
                        q_bus_wrdata <= alu_result;
                end
            end
        end
    end

    //-------------------------------------------------------------------------
    // BC('), DE('), HL('), IX and IY
    //-------------------------------------------------------------------------
    reg   [2:0] q_reg_idx_a;
    reg   [2:0] q_reg_idx_b;
    reg   [2:0] q_reg_idx_c;

    always @(posedge clk) begin
        if (clk_en) begin
            // Bus A / Write
            q_reg_idx_a <= {q_regs_alt,dec_set_bus_a_to[2:1]};
            if (!q_xy_ind && q_xy_state != 2'b00 && dec_set_bus_a_to[2:1] == 2'b10)
                q_reg_idx_a <= {q_xy_state[1], 2'b11};

            // Bus B
            q_reg_idx_b <= {q_regs_alt,dec_set_bus_b_to[2:1]};
            if (!q_xy_ind && q_xy_state != 2'b00 && dec_set_bus_b_to[2:1] == 2'b10)
                q_reg_idx_b <= {q_xy_state[1], 2'b11};

            // Address from register
            q_reg_idx_c <= {q_regs_alt,dec_set_addr_to[1:0]};

            // dec_jump (HL), LD SP,HL
            if (dec_is_jp_ind_hl || dec_is_ldsphl)
                q_reg_idx_c <= {q_regs_alt,2'b10};
            if (((dec_is_jp_ind_hl || dec_is_ldsphl) && q_xy_state != 2'b00) || q_mcycle == 3'd6)
                q_reg_idx_c <= {q_xy_state[1], 2'b11};

            if (dec_is_djnz && q_save_alu)
                q_inc_dec_is_zero <= d_reg_f[Flag_Z];
            if ((q_tstate == 2 || (q_tstate == 3 && q_mcycle == 3'd1)) && dec_incdec16[2:0] == 3'd4)
                q_inc_dec_is_zero <= (incdec16_result != 16'b0);

            q_reg_bus_a <= reg_bus_a;
        end
    end

    reg [2:0] reg_idx_a;
    always @* begin
        if      ((q_tstate == 2 || (q_tstate == 3 && q_mcycle == 3'd1 && dec_incdec16[2])) && q_xy_state == 2'b00)
            reg_idx_a = {q_regs_alt, dec_incdec16[1:0]};
        else if ((q_tstate == 2 || (q_tstate == 3 && q_mcycle == 3'd1 && dec_incdec16[2])) && dec_incdec16[1:0] == 2'b10)
            reg_idx_a = {q_xy_state[1], 2'b11};
        else if (q_tstate == 3 && dec_is_ex_de_hl)
            reg_idx_a = {q_regs_alt, 2'b10};
        else if (q_tstate == 4 && dec_is_ex_de_hl)
            reg_idx_a = {q_regs_alt, 2'b01};
        else if (q_tstate == 4 && dec_exchange_wh)
            reg_idx_a = (q_xy_state == 2'b00) ? {q_regs_alt, 2'b10} : {q_xy_state[1], 2'b11};
        else
            reg_idx_a = q_reg_idx_a;
    end

    wire [2:0] reg_idx_b = dec_is_ex_de_hl && q_tstate == 3 ? {q_regs_alt,2'b01} : q_reg_idx_b;
    assign incdec16_result = dec_incdec16[3] ? (reg_bus_a - 16'd1) : (reg_bus_a + 16'd1);

    always @* begin
        reg_wren_h = 0;
        reg_wren_l = 0;
        if ((q_tstate == 1 && !q_save_alu && !q_auto_wait) || (q_save_alu && q_alu_op != AluOpCp)) begin
            case (q_read_to_reg)
                5'b10000,5'b10001,5'b10010,5'b10011,5'b10100,5'b10101: begin
                    reg_wren_h = ~q_read_to_reg[0];
                    reg_wren_l =  q_read_to_reg[0];
                end
                default: begin end
            endcase
        end
        if (dec_is_ex_de_hl && (q_tstate == 3 || q_tstate == 4)) begin
            reg_wren_h = 1;
            reg_wren_l = 1;
        end
        if (dec_exchange_wh && q_tstate == 4) begin
            reg_wren_h = 1;
            reg_wren_l = 1;
        end
        if (dec_incdec16[2] && dec_incdec16[1:0] != 2'b11 &&
            ((q_tstate == 2 && q_mcycle != 3'd1 && !really_wait) ||
             (q_tstate == 3 && q_mcycle == 3'd1))) begin

            reg_wren_h = 1;
            reg_wren_l = 1;
        end
    end

    reg [15:0] reg_wrdata;
    always @* begin
        reg_wrdata = {save_mux, save_mux};
        if (dec_is_ex_de_hl && q_tstate == 3)      reg_wrdata = reg_bus_b;
        if (dec_is_ex_de_hl && q_tstate == 4)      reg_wrdata = q_reg_bus_a;
        if (dec_exchange_wh && q_tstate == 4)      reg_wrdata = q_memptr;
        if (dec_incdec16[2] &&
            ((q_tstate == 2 && q_mcycle != 3'd1) ||
             (q_tstate == 3 && q_mcycle == 3'd1))) reg_wrdata = incdec16_result;
    end

    //------------------------------------------------------------------------
    // Register file
    //------------------------------------------------------------------------
    reg [7:0] regs_h [7:0] /* synthesis syn_ramstyle = "distributed_ram" */;
    reg [7:0] regs_l [7:0] /* synthesis syn_ramstyle = "distributed_ram" */;

    always @(posedge clk) if (clk_en && reg_wren_h) regs_h[reg_idx_a] <= reg_wrdata[15:8];
    always @(posedge clk) if (clk_en && reg_wren_l) regs_l[reg_idx_a] <= reg_wrdata[7:0];

    assign reg_bus_a[15:8] = regs_h[reg_idx_a];
    assign reg_bus_a[ 7:0] = regs_l[reg_idx_a];

    assign reg_bus_b[15:8] = regs_h[reg_idx_b];
    assign reg_bus_b[ 7:0] = regs_l[reg_idx_b];

    assign reg_bus_c[15:8] = regs_h[q_reg_idx_c];
    assign reg_bus_c[ 7:0] = regs_l[q_reg_idx_c];

    //------------------------------------------------------------------------
    // Buses
    //------------------------------------------------------------------------
    always @(posedge clk) begin
        if (clk_en) begin
            case (dec_set_bus_b_to)
                4'b0111: bus_b <= q_reg_a;
                4'b0000,
                4'b0001,
                4'b0010,
                4'b0011,
                4'b0100,
                4'b0101: bus_b <= dec_set_bus_b_to[0] ? reg_bus_b[7:0] : reg_bus_b[15:8];
                4'b0110: bus_b <= q_di;
                4'b1000: bus_b <= q_reg_sp[7:0];
                4'b1001: bus_b <= q_reg_sp[15:8];
                4'b1010: bus_b <= 8'h01;
                4'b1011: bus_b <= q_reg_f;
                4'b1100: bus_b <= q_reg_pc[7:0];
                4'b1101: bus_b <= q_reg_pc[15:8];
                4'b1110: bus_b <= 8'h00;
                default: bus_b <= 8'h00;
            endcase

            case (dec_set_bus_a_to)
                4'b0111: bus_a <= q_reg_a;
                4'b0000,
                4'b0001,
                4'b0010,
                4'b0011,
                4'b0100,
                4'b0101: bus_a <= dec_set_bus_a_to[0] ? reg_bus_a[7:0] : reg_bus_a[15:8];
                4'b0110: bus_a <= q_di;
                4'b1000: bus_a <= q_reg_sp[7:0];
                4'b1001: bus_a <= q_reg_sp[15:8];
                4'b1010: bus_a <= 8'h00;
                default: bus_a <= 8'h00;
            endcase

            if (dec_xybit_undoc) begin
                bus_a <= q_di;
                bus_b <= q_di;
            end
        end
    end

    //------------------------------------------------------------------------
    // Main state machine
    //------------------------------------------------------------------------
    reg       q_nmi;
    reg       q_nmi_pending;
    reg       q2_auto_wait;
    reg [2:0] q_pre_xy_f_m;

    always @(posedge clk) begin
        if (reset) begin
            q_mcycle      <= 3'd1;
            q_tstate      <= 3'd0;
            q_pre_xy_f_m  <= 3'd0;
            q_halt        <= 0;
            q_nmi_cycle   <= 0;
            q_irq_cycle   <= 0;
            q_int_en1     <= 0;
            q_int_en2     <= 0;
            q_no_btr      <= 0;
            q_auto_wait   <= 0;
            q2_auto_wait  <= 0;
            q_nmi_pending <= 0;
            q_nmi         <= 0;

        end else begin
            q_nmi <= nmi;
            if (nmi && !q_nmi) begin
                q_nmi_pending <= 1;
            end

            if (clk_en) begin
                q2_auto_wait <= q_auto_wait;

                if (t_reset) begin
                    q_auto_wait  <= 0;
                    q2_auto_wait <= 0;
                end else begin
                    q_auto_wait  <= d_auto_wait | dec_iorq;
                end

                q_no_btr <=
                    (dec_is_bt  & (~q_instruction[4] |                    ~q_reg_f[Flag_P])) |
                    (dec_is_bc  & (~q_instruction[4] |  q_reg_f[Flag_Z] | ~q_reg_f[Flag_P])) |
                    (dec_is_btr & (~q_instruction[4] |  q_reg_f[Flag_Z]));

                if (q_tstate == 2) begin
                    if (dec_is_ei) begin
                        q_int_en1 <= 1;
                        q_int_en2 <= 1;
                    end

                    if (dec_is_retn)
                        q_int_en1 <= q_int_en2;
                end

                if (q_tstate == 3 && dec_is_di) begin
                    q_int_en1 <= 0;
                    q_int_en2 <= 0;
                end

                if (q_irq_cycle || q_nmi_cycle)
                    q_halt <= 0;

                if (q_tstate == 2 && really_wait) begin
                    // Wait

                end else if (t_reset) begin
                    if (dec_is_halt) begin
                        q_halt <= 1;
                    end
                    q_tstate <= 3'd1;

                    if (next_is_xy_fetch) begin
                        q_mcycle     <= 3'd6;
                        q_pre_xy_f_m <= q_instruction == 8'h36 ? 3'd2 : q_mcycle;

                    end else if (q_mcycle == 3'd7) begin
                        q_mcycle <= q_pre_xy_f_m + 3'd1;

                    end else if (q_mcycle == q_mcycles || q_no_btr || (q_mcycle == 3'd2 && dec_is_djnz && q_inc_dec_is_zero)) begin
                        q_mcycle    <= 3'd1;
                        q_irq_cycle <= 0;
                        q_nmi_cycle <= 0;

                        if (q_nmi_pending && dec_prefix == PrefixNone) begin
                            q_nmi_pending <= 0;
                            q_nmi_cycle   <= 1;
                            q_int_en1     <= 0;

                        end else if (q_int_en1 && irq && dec_prefix == PrefixNone && !dec_is_ei) begin
                            q_irq_cycle <= 1;
                            q_int_en1   <= 0;
                            q_int_en2   <= 0;
                        end

                    end else begin
                        q_mcycle <= q_mcycle + 1;
                    end

                end else if (!(d_auto_wait && !q2_auto_wait)) begin
                    q_tstate <= q_tstate + 1;
                end
            end
        end
    end

    assign d_auto_wait = q_irq_cycle && q_mcycle == 3'd1;

endmodule
