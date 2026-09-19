`default_nettype none
`timescale 1 ns / 1 ps

module cpu #(
    parameter VEC_RESET    = 32'hFF100000,
    parameter IRQ_USED     = 32'h00000000,
    parameter IRQ_LATCHING = 32'h00000000
) (
    input  wire        clk,
    input  wire        reset,

    input  wire [63:0] mtime,

    // Bus interface
    output wire [31:0] bus_addr,
    output wire [31:0] bus_wrdata,
    output wire  [3:0] bus_bytesel,
    output wire        bus_wren,
    output wire        bus_strobe,
    input  wire        bus_wait,
    input  wire [31:0] bus_rddata,

    // Interrupt input
    input  wire [31:0] irq);

    localparam [1:0]
        StFetch = 2'd0,
        StDiv   = 2'd1,
        StMemRd = 2'd2,
        StMemWr = 2'd3;

    localparam [4:0]
        TrapIllegalInstruction  = 5'd2,
        TrapBreakpoint          = 5'd3,
        TrapLoadAddrMisaligned  = 5'd4,
        TrapStoreAddrMisaligned = 5'd6,
        TrapEcallM              = 5'd11;

    reg [31:0] d_pc,            q_pc;
    reg [31:0] d_instr,         q_instr;
    reg  [1:0] d_state,         q_state;

    // Bus interface
    reg [31:0] d_addr,          q_addr;
    reg [31:0] d_wrdata,        q_wrdata;
    reg  [3:0] d_bytesel,       q_bytesel;
    reg        d_wren,          q_wren;
    reg        d_stb,           q_stb;

    // CSRs
    reg        d_mstatus_mie,   q_mstatus_mie;  // 0x300 Machine status register:  <3> Machine interrupt enable
    reg        d_mstatus_mpie,  q_mstatus_mpie; // 0x300 Machine status register:  <7> Machine pre-trap interrupt enable
    reg [31:0] d_mie,           q_mie;          // 0x304 Machine interrupt-enable register
    reg [31:0] d_mtvec,         q_mtvec;        // 0x305 Machine trap-handler base address
    reg [31:0] d_mscratch,      q_mscratch;     // 0x340 Scratch register for machine trap handlers
    reg [31:0] d_mepc,          q_mepc;         // 0x341 Machine exception program counter
    reg        d_mcause_irq,    q_mcause_irq;   // 0x342 Machine trap cause
    reg  [4:0] d_mcause_code,   q_mcause_code;  // 0x342 Machine trap cause
    reg [31:0] d_mtval,         q_mtval;        // 0x343 Machine bad address or instruction
    reg [31:0] d_mip,           q_mip;          // 0x344 Machine interrupt-pending register
    reg [63:0] d_mcycle,        q_mcycle;       // 0xB00/0xB80 Machine cycle counter

    assign bus_addr     = q_addr;
    assign bus_wrdata   = q_wrdata;
    assign bus_bytesel  = q_bytesel;
    assign bus_wren     = q_wren;
    assign bus_strobe   = q_stb;

    //////////////////////////////////////////////////////////////////////////
    // Instruction decoding
    //////////////////////////////////////////////////////////////////////////
    wire  [6:0] opcode  = d_instr[6:0];
    wire  [2:0] funct3  = d_instr[14:12];
    wire  [6:0] funct7  = d_instr[31:25];
    wire  [4:0] rs1_idx = d_instr[19:15];
    wire  [4:0] rs2_idx = d_instr[24:20];
    wire  [4:0] rd_idx  = d_instr[11:7];
    wire [31:0] imm_i   = {{21{d_instr[31]}}, d_instr[30:20]};
    wire [31:0] imm_s   = {{21{d_instr[31]}}, d_instr[30:25], d_instr[11:7]};
    wire [31:0] imm_b   = {{20{d_instr[31]}}, d_instr[7], d_instr[30:25], d_instr[11:8], 1'b0};
    wire [31:0] imm_u   = {d_instr[31:12], 12'b0};
    wire [31:0] imm_j   = {{12{d_instr[31]}}, d_instr[19:12], d_instr[20], d_instr[30:21], 1'b0};
    wire [11:0] csr     = d_instr[31:20];

    wire is_lui     = (opcode == 7'b0110111); // regfile[rd_idx] = imm_u
    wire is_auipc   = (opcode == 7'b0010111); // regfile[rd_idx] = pc + imm_u
    wire is_jal     = (opcode == 7'b1101111); // regfile[rd_idx] = pc + 4; pc += imm_j
    wire is_jalr    = (opcode == 7'b1100111); // regfile[rd_idx] = pc + 4; pc = regfile[rs1_idx] + imm_i
    wire is_branch  = (opcode == 7'b1100011); // if (regfile[rs1_idx] <branch_op> regfile[rs2_idx]) pc += imm_b
    wire is_load    = (opcode == 7'b0000011); // regfile[rd_idx] = mem[regfile[rs1_idx] + imm_i]
    wire is_store   = (opcode == 7'b0100011); // mem[regfile[rs1_idx] + imm_s] = regfile[rs2_idx]
    wire is_alu_imm = (opcode == 7'b0010011); // regfile[rd_idx] = regfile[rs1_idx] <alu_op> imm_i
    wire is_alu_reg = (opcode == 7'b0110011); // regfile[rd_idx] = regfile[rs1_idx] <alu_op> regfile[rs2_idx]
    wire is_system  = (opcode == 7'b1110011); // ECALL/EBREAK
    wire is_fence   = (opcode == 7'b0001111 && funct3 == 3'b000); // FENCE/PAUSE

    wire is_ecall   = is_system && d_instr[31:7] == 25'b0000000_00000_00000_000_00000;
    wire is_ebreak  = is_system && d_instr[31:7] == 25'b0000000_00001_00000_000_00000;
    wire is_mret    = is_system && d_instr[31:7] == 25'b0011000_00010_00000_000_00000;
    wire is_csr     = is_system && funct3[1:0] != 2'b00;

    wire is_valid_instruction =
        is_lui     ||
        is_auipc   ||
        is_jal     ||
        is_jalr    ||
        is_branch  ||
        is_load    ||
        is_store   ||
        is_alu_imm ||
        is_alu_reg ||
        is_fence   ||
        is_system  ||
        is_ecall   ||
        is_ebreak;

    //////////////////////////////////////////////////////////////////////////
    // Register file
    //////////////////////////////////////////////////////////////////////////
    reg [31:0] rd_data;
    reg        rd_wr;
    reg [31:0] regfile [31:0] /* synthesis syn_ramstyle = "distributed_ram" */;

    always @(posedge clk) if (rd_wr && rd_idx != 5'd0) regfile[rd_idx] <= rd_data;
    wire [31:0] rs1_data = regfile[rs1_idx];
    wire [31:0] rs2_data = regfile[rs2_idx];

    wire signed [31:0] rs2_data_s = rs2_data;

`ifdef MODEL_TECH
    wire [31:0] reg0_zero  = regfile[0];
    wire [31:0] reg1_ra    = regfile[1];
    wire [31:0] reg2_sp    = regfile[2];
    wire [31:0] reg3_gp    = regfile[3];
    wire [31:0] reg4_tp    = regfile[4];
    wire [31:0] reg5_t0    = regfile[5];
    wire [31:0] reg6_t1    = regfile[6];
    wire [31:0] reg7_t2    = regfile[7];
    wire [31:0] reg8_s0_fp = regfile[8];
    wire [31:0] reg9_s1    = regfile[9];
    wire [31:0] reg10_a0   = regfile[10];
    wire [31:0] reg11_a1   = regfile[11];
    wire [31:0] reg12_a2   = regfile[12];
    wire [31:0] reg13_a3   = regfile[13];
    wire [31:0] reg14_a4   = regfile[14];
    wire [31:0] reg15_a5   = regfile[15];
    wire [31:0] reg16_a6   = regfile[16];
    wire [31:0] reg17_a7   = regfile[17];
    wire [31:0] reg18_s2   = regfile[18];
    wire [31:0] reg19_s3   = regfile[19];
    wire [31:0] reg20_s4   = regfile[20];
    wire [31:0] reg21_s5   = regfile[21];
    wire [31:0] reg22_s6   = regfile[22];
    wire [31:0] reg23_s7   = regfile[23];
    wire [31:0] reg24_s8   = regfile[24];
    wire [31:0] reg25_s9   = regfile[25];
    wire [31:0] reg26_s10  = regfile[26];
    wire [31:0] reg27_s11  = regfile[27];
    wire [31:0] reg28_t3   = regfile[28];
    wire [31:0] reg29_t4   = regfile[29];
    wire [31:0] reg30_t5   = regfile[30];
    wire [31:0] reg31_t6   = regfile[31];

    integer i;
    initial begin
        regfile[0] = 0;
        for (i=1; i<32; i=i+1)
            regfile[i] = 32'hDEAD0000 + i;

        regfile[1] = 32'hDEADC0DE;
        regfile[2] = 32'h00000000;

    end
`endif

    //////////////////////////////////////////////////////////////////////////
    // ALU
    //////////////////////////////////////////////////////////////////////////

    // ALU operands
    wire [31:0] l_operand = rs1_data;
    wire [31:0] r_operand = (is_alu_reg || is_branch) ? rs2_data : imm_i;
    wire  [4:0] shamt     = is_alu_imm ? rs2_idx : rs2_data[4:0];

    // Add/sub
    wire [31:0] alu_add = l_operand + r_operand;
    wire [32:0] alu_sub = {1'b0, l_operand} - {1'b0, r_operand};

    wire        is_eq  = alu_sub[31:0] == 32'h0;
    wire        is_lt  = (l_operand[31] ^ r_operand[31]) ? l_operand[31] : alu_sub[32];
    wire        is_ltu = alu_sub[32];

    // Left barrel-shifter
    wire [31:0] shl0  = l_operand;
    wire [31:0] shl1  = shamt[0] ? {shl0[30:0],  1'b0} : shl0;
    wire [31:0] shl2  = shamt[1] ? {shl1[29:0],  2'b0} : shl1;
    wire [31:0] shl4  = shamt[2] ? {shl2[27:0],  4'b0} : shl2;
    wire [31:0] shl8  = shamt[3] ? {shl4[23:0],  8'b0} : shl4;
    wire [31:0] shl16 = shamt[4] ? {shl8[15:0], 16'b0} : shl8;

    // Right barrel-shifter
    wire shr_msb = funct7[5] ? l_operand[31] : 1'b0;
    wire [31:0] shr0  = l_operand;
    wire [31:0] shr1  = shamt[0] ? {{ 1{shr_msb}}, shr0[31: 1]} : shr0;
    wire [31:0] shr2  = shamt[1] ? {{ 2{shr_msb}}, shr1[31: 2]} : shr1;
    wire [31:0] shr4  = shamt[2] ? {{ 4{shr_msb}}, shr2[31: 4]} : shr2;
    wire [31:0] shr8  = shamt[3] ? {{ 8{shr_msb}}, shr4[31: 8]} : shr4;
    wire [31:0] shr16 = shamt[4] ? {{16{shr_msb}}, shr8[31:16]} : shr8;

    // Multiplier
    wire signed [32:0] mult_l = {rs1_data[31] && (funct3 == 3'b001 || funct3 == 3'b010), rs1_data};
    wire signed [32:0] mult_r = {rs2_data[31] && (funct3 == 3'b001), rs2_data};
    wire signed [63:0] mult_result = mult_l * mult_r;

    wire is_mul_div = is_alu_reg && funct7[0];
    wire is_div_rem = is_mul_div && funct3[2];

    // Divider
    reg  [31:0] q_quotient;
    reg  [31:0] q_remainder;
    reg         q_div_busy;
    reg         div_start;
    wire        div_is_signed = !funct3[0] && is_div_rem;

    reg  [31:0] q_div_dividend;
    reg  [62:0] q_div_divisor;
    reg  [31:0] q_div_quotient;
    reg  [31:0] q_div_quotient_msk;

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_div_busy         <= 0;
            q_quotient         <= 0;
            q_remainder        <= 0;
            q_div_dividend     <= 0;
            q_div_divisor      <= 0;
            q_div_quotient     <= 0;
            q_div_quotient_msk <= 0;

        end else begin
            if (div_start) begin
                q_div_busy         <= 1;
                q_div_dividend     <=  (div_is_signed && rs1_data[31]) ? -rs1_data : rs1_data;
                q_div_divisor      <= {(div_is_signed && rs2_data[31]) ? -rs2_data : rs2_data, 31'b0};
                q_div_quotient     <= 0;
                q_div_quotient_msk <= 32'h80000000;

            end else if (q_div_quotient_msk == 0 && q_div_busy) begin
                q_div_busy  <= 0;
                q_quotient  <= (div_is_signed && rs1_data[31] != rs2_data[31] && rs2_data != 32'b0) ? -q_div_quotient : q_div_quotient;
                q_remainder <= (div_is_signed && rs1_data[31]) ? -q_div_dividend : q_div_dividend;

            end else begin
                if (q_div_divisor <= {31'b0, q_div_dividend}) begin
                    q_div_dividend <= q_div_dividend - q_div_divisor[31:0];
                    q_div_quotient <= q_div_quotient | q_div_quotient_msk;
                end

                q_div_divisor      <= {1'b0, q_div_divisor[62:1]};
                q_div_quotient_msk <= {1'b0, q_div_quotient_msk[31:1]};
            end
        end

    // Operation selection
    reg [31:0] alu_result;
    always @* begin
        if (is_mul_div) begin
            case (funct3)
                3'b000: alu_result = mult_result[31:0];     // MUL
                3'b001: alu_result = mult_result[63:32];    // MULH
                3'b010: alu_result = mult_result[63:32];    // MULHSU
                3'b011: alu_result = mult_result[63:32];    // MULHU
                3'b100: alu_result = q_quotient;            // DIV
                3'b101: alu_result = q_quotient;            // DIVU
                3'b110: alu_result = q_remainder;           // REM
                3'b111: alu_result = q_remainder;           // REMU
            endcase
        end else begin
            case (funct3)
                3'b000: alu_result = is_alu_reg && funct7[5] ? alu_sub[31:0] : alu_add;   // ADD
                3'b001: alu_result = shl16;                                               // SLL (shift left logical)
                3'b010: alu_result = {31'b0, is_lt};                                      // SLT
                3'b011: alu_result = {31'b0, is_ltu};                                     // SLTU
                3'b100: alu_result = l_operand ^ r_operand;                               // XOR
                3'b101: alu_result = shr16;                                               // SRL / SRA (shift right logical / arithmetic)
                3'b110: alu_result = l_operand | r_operand;                               // OR
                3'b111: alu_result = l_operand & r_operand;                               // AND
            endcase
        end
    end

    // Load/store address
    wire [31:0] load_store_addr = rs1_data + (is_load ? imm_i : imm_s);

    // Branch condition
    reg do_branch;
    always @* begin
        do_branch = 1'b0;
        if (is_branch) begin
            case (funct3)
                3'b000:  do_branch =  is_eq;
                3'b001:  do_branch = !is_eq;
                3'b100:  do_branch =  is_lt;
                3'b101:  do_branch = !is_lt;
                3'b110:  do_branch =  is_ltu;
                3'b111:  do_branch = !is_ltu;
                default: do_branch = 1'b0;
            endcase
        end
    end

    //////////////////////////////////////////////////////////////////////////
    // Load data logic
    //////////////////////////////////////////////////////////////////////////
    reg [7:0] lb_data;
    always @* case (load_store_addr[1:0])
        2'b00: lb_data = bus_rddata[7:0];
        2'b01: lb_data = bus_rddata[15:8];
        2'b10: lb_data = bus_rddata[23:16];
        2'b11: lb_data = bus_rddata[31:24];
    endcase

    wire [15:0] lh_data = load_store_addr[1] ? bus_rddata[31:16] : bus_rddata[15:0];

    reg [31:0] load_data;
    always @* begin
        if      (funct3[1]) load_data = bus_rddata;                                                  // LW
        else if (funct3[0]) load_data = funct3[2] ? {16'b0, lh_data} : {{16{lh_data[15]}}, lh_data}; // LH/LHU
        else                load_data = funct3[2] ? {24'b0, lb_data} : {{24{lb_data[ 7]}}, lb_data}; // LB/LBU
    end

    wire signed [31:0] bus_rddata_s = bus_rddata;

    //////////////////////////////////////////////////////////////////////////
    // CSR
    //////////////////////////////////////////////////////////////////////////

    // Machine Trap Setup
    wire is_mstatus  = (csr == 12'h300);    // Machine status register
    wire is_mie      = (csr == 12'h304);    // Machine interrupt-enable register
    wire is_mtvec    = (csr == 12'h305);    // Machine trap-handler base address

    // Machine Trap Handling
    wire is_mscratch = (csr == 12'h340);    // Scratch register for machine trap handlers
    wire is_mepc     = (csr == 12'h341);    // Machine exception program counter
    wire is_mcause   = (csr == 12'h342);    // Machine trap cause
    wire is_mtval    = (csr == 12'h343);    // Machine bad address or instruction
    wire is_mip      = (csr == 12'h344);    // Machine interrupt pending

    // Machine Counter/Timers
    wire is_mcycle   = (csr == 12'hB00 || csr == 12'hC00);    // Machine cycle counter
    wire is_mcycleh  = (csr == 12'hB80 || csr == 12'hC80);    // Upper 32 bits of mcycle
    wire is_time     = (csr == 12'hC01);                      // Machine time counter
    wire is_timeh    = (csr == 12'hC81);                      // Upper 32 bits of mtime

    // mstatus register
    reg [31:0] mstatus;
    always @* begin
        mstatus     = 0;
        mstatus[12] = 1;
        mstatus[11] = 1;
        mstatus[7]  = q_mstatus_mpie;
        mstatus[3]  = q_mstatus_mie;
    end

    reg [31:0] csr_rdata;
    always @* begin
        csr_rdata = 32'h00000000;
        if (is_mstatus)   csr_rdata = mstatus;
        if (is_mie)       csr_rdata = q_mie & IRQ_USED;
        if (is_mtvec)     csr_rdata = q_mtvec;
        if (is_mscratch)  csr_rdata = q_mscratch;
        if (is_mepc)      csr_rdata = {q_mepc[31:2], 2'b0};
        if (is_mcause)    csr_rdata = {q_mcause_irq, 26'b0, q_mcause_code};
        if (is_mtval)     csr_rdata = q_mtval;
        if (is_mip)       csr_rdata = q_mip & IRQ_USED;
        if (is_mcycle)    csr_rdata = q_mcycle[31:0];
        if (is_mcycleh)   csr_rdata = q_mcycle[63:32];
        if (is_time)      csr_rdata = mtime[31:0];
        if (is_timeh)     csr_rdata = mtime[63:32];
    end

    wire [31:0] csr_operand = funct3[2] ? {27'd0, rs1_idx} : rs1_data;
    reg  [31:0] csr_wdata;
    always @*
        if      (funct3[1:0] == 2'b10) csr_wdata = csr_rdata |  csr_operand;   // CSRRS(I) set-bits
        else if (funct3[1:0] == 2'b11) csr_wdata = csr_rdata & ~csr_operand;   // CSRRC(I) clear-bits
        else                           csr_wdata =              csr_operand;   // CSRRW(I) assign

    // For both CSRRS and CSRRC, if rs1=x0, then the instruction will not write to the CSR at all.
    wire csr_write = is_csr && !(funct3[2:1] == 2'b01 && rs1_idx == 5'd0);

    //////////////////////////////////////////////////////////////////////////
    // Interrupts
    //////////////////////////////////////////////////////////////////////////
    reg [4:0] irq_code;
    always @* begin
        irq_code = 0;
        if (IRQ_USED[31] && q_mip[31]) irq_code = 5'd31;
        if (IRQ_USED[30] && q_mip[30]) irq_code = 5'd30;
        if (IRQ_USED[29] && q_mip[29]) irq_code = 5'd29;
        if (IRQ_USED[28] && q_mip[28]) irq_code = 5'd28;
        if (IRQ_USED[27] && q_mip[27]) irq_code = 5'd27;
        if (IRQ_USED[26] && q_mip[26]) irq_code = 5'd26;
        if (IRQ_USED[25] && q_mip[25]) irq_code = 5'd25;
        if (IRQ_USED[24] && q_mip[24]) irq_code = 5'd24;
        if (IRQ_USED[23] && q_mip[23]) irq_code = 5'd23;
        if (IRQ_USED[22] && q_mip[22]) irq_code = 5'd22;
        if (IRQ_USED[21] && q_mip[21]) irq_code = 5'd21;
        if (IRQ_USED[20] && q_mip[20]) irq_code = 5'd20;
        if (IRQ_USED[19] && q_mip[19]) irq_code = 5'd19;
        if (IRQ_USED[18] && q_mip[18]) irq_code = 5'd18;
        if (IRQ_USED[17] && q_mip[17]) irq_code = 5'd17;
        if (IRQ_USED[16] && q_mip[16]) irq_code = 5'd16;
        if (IRQ_USED[15] && q_mip[15]) irq_code = 5'd15;
        if (IRQ_USED[14] && q_mip[14]) irq_code = 5'd14;
        if (IRQ_USED[13] && q_mip[13]) irq_code = 5'd13;
        if (IRQ_USED[12] && q_mip[12]) irq_code = 5'd12;
        if (IRQ_USED[11] && q_mip[11]) irq_code = 5'd11;
        if (IRQ_USED[10] && q_mip[10]) irq_code = 5'd10;
        if (IRQ_USED[ 9] && q_mip[ 9]) irq_code = 5'd9;
        if (IRQ_USED[ 8] && q_mip[ 8]) irq_code = 5'd8;
        if (IRQ_USED[ 7] && q_mip[ 7]) irq_code = 5'd7;
        if (IRQ_USED[ 6] && q_mip[ 6]) irq_code = 5'd6;
        if (IRQ_USED[ 5] && q_mip[ 5]) irq_code = 5'd5;
        if (IRQ_USED[ 4] && q_mip[ 4]) irq_code = 5'd4;
        if (IRQ_USED[ 3] && q_mip[ 3]) irq_code = 5'd3;
        if (IRQ_USED[ 2] && q_mip[ 2]) irq_code = 5'd2;
        if (IRQ_USED[ 1] && q_mip[ 1]) irq_code = 5'd1;
        if (IRQ_USED[ 0] && q_mip[ 0]) irq_code = 5'd0;
    end

    wire irq_pending = (q_mip & q_mie) != 0;

    //////////////////////////////////////////////////////////////////////////
    // State machine
    //////////////////////////////////////////////////////////////////////////
    wire [31:0] pc_plus4    = q_pc + 32'd4;
    wire [31:0] pc_plus_imm = q_pc + (is_jal ? imm_j : (is_auipc ? imm_u : imm_b));

    always @* begin
        rd_data = 32'b0;
        if      (is_lui)                        rd_data = imm_u;
        else if (is_auipc)                      rd_data = pc_plus_imm;
        else if (is_jal || is_jalr)             rd_data = pc_plus4;
        else if (is_load)                       rd_data = load_data;
        else if (is_alu_imm || is_alu_reg)      rd_data = alu_result;
        else if (is_csr)                        rd_data = csr_rdata;
    end

    reg        do_trap;
    reg        trap_is_irq;
    reg  [4:0] trap_code;
    reg [31:0] trap_mtval;

    task trap;
        input        is_irq;
        input  [4:0] code;
        input [31:0] mtval;

        begin
            do_trap     = 1;
            trap_is_irq = is_irq;
            trap_code   = code;
            trap_mtval  = mtval;
        end
    endtask

    task fetch;
        input [31:0] newpc;

        begin
            d_state      = StFetch;
            d_pc         = {newpc[31:2], 2'b00};
            d_addr       = {newpc[31:2], 2'b00};
            d_bytesel    = 4'b1111;
            d_wren       = 0;
            d_stb        = 1;
        end
    endtask

    always @* begin
        d_pc           = q_pc;
        d_instr        = q_instr;
        d_state        = q_state;
        d_addr         = q_addr;
        d_wrdata       = q_wrdata;
        d_bytesel      = q_bytesel;
        d_wren         = q_wren;
        d_stb          = q_stb;
        d_mstatus_mie  = q_mstatus_mie;
        d_mstatus_mpie = q_mstatus_mpie;
        d_mie          = q_mie;
        d_mtvec        = q_mtvec;
        d_mscratch     = q_mscratch;
        d_mepc         = q_mepc;
        d_mcause_irq   = q_mcause_irq;
        d_mcause_code  = q_mcause_code;
        d_mtval        = q_mtval;
        d_mip          = q_mip;
        d_mcycle       = q_mcycle + 64'd1;
        rd_wr          = 0;
        div_start      = 0;
        do_trap        = 0;
        trap_is_irq    = 0;
        trap_code      = 0;
        trap_mtval     = 0;

        case (q_state)
            StFetch: begin
                if (!bus_wait) begin
                    d_wren     = 0;
                    d_stb      = 0;

                    if (irq_pending && q_mstatus_mie) begin
                        trap(1, irq_code, 0);

                    end else begin
                        d_instr = bus_rddata;

                        begin
                            if (!is_valid_instruction) begin
                                trap(0, TrapIllegalInstruction, 0);

                            end else if (is_ebreak || is_ecall) begin
                                if (is_ebreak)
                                    trap(0, TrapBreakpoint, 0);
                                else if (is_ecall)
                                    trap(0, TrapEcallM, 0);

                            end else if (is_mret) begin
                                d_mstatus_mie  = q_mstatus_mpie;
                                d_mstatus_mpie = 1;

                                fetch(q_mepc);

                            end else if (is_div_rem) begin
                                div_start = 1;
                                d_state   = StDiv;

                            end else if (is_load || is_store) begin
                                // Check load/store alignment
                                if (funct3[1] ? (load_store_addr[1:0] != 0) : (funct3[0] ? (load_store_addr[0] != 0) : 0))
                                    trap(0, is_load ? TrapLoadAddrMisaligned : TrapStoreAddrMisaligned, load_store_addr);

                                d_state = is_load ? StMemRd : StMemWr;
                                d_addr  = load_store_addr;
                                d_wren  = is_store;
                                d_stb   = 1;

                                if      (funct3[1]) d_wrdata = rs2_data;                                                     // SW
                                else if (funct3[0]) d_wrdata = {rs2_data[15:0], rs2_data[15:0]};                             // SH
                                else                d_wrdata = {rs2_data[7:0], rs2_data[7:0], rs2_data[7:0], rs2_data[7:0]}; // SB

                                if (is_store) begin
                                    if (funct3[1])          // SW
                                        d_bytesel = 4'b1111;
                                    else if (funct3[0])     // SH
                                        d_bytesel = load_store_addr[1] ? 4'b1100 : 4'b0011;
                                    else begin              // SB
                                        case (load_store_addr[1:0])
                                            2'b00: d_bytesel = 4'b0001;
                                            2'b01: d_bytesel = 4'b0010;
                                            2'b10: d_bytesel = 4'b0100;
                                            2'b11: d_bytesel = 4'b1000;
                                        endcase
                                    end
                                end

                            end else begin
                                if (is_lui || is_auipc || is_jal || is_jalr || is_alu_imm || is_alu_reg || is_system)
                                    rd_wr = 1;

                                // CSR write handling
                                if (csr_write) begin
                                    if (is_mstatus) begin
                                        d_mstatus_mpie = csr_wdata[7];
                                        d_mstatus_mie  = csr_wdata[3];
                                    end

                                    if (is_mie)      d_mie           = csr_wdata & IRQ_USED;
                                    if (is_mtvec)    d_mtvec         = {csr_wdata[31:2], 2'b0};
                                    if (is_mscratch) d_mscratch      = csr_wdata;
                                    if (is_mepc)     d_mepc[31:2]    = csr_wdata[31:2];
                                    if (is_mip)      d_mip           = csr_wdata & IRQ_USED;
                                    if (is_mcycle)   d_mcycle[31:0]  = csr_wdata;
                                    if (is_mcycleh)  d_mcycle[63:32] = csr_wdata;

                                    if (is_mcause) begin
                                        d_mcause_irq  = csr_wdata[31];
                                        d_mcause_code = csr_wdata[4:0];
                                    end

                                    if (is_mtval) d_mtval = csr_wdata;
                                end

                                if      (is_jal || do_branch) fetch(pc_plus_imm);
                                else if (is_jalr)             fetch(alu_add);
                                else                          fetch(pc_plus4);
                            end
                        end
                    end
                end
            end

            StDiv: begin
                // Wait for division to be done
                if (!q_div_busy) begin
                    rd_wr      = 1;
                    fetch(pc_plus4);
                end
            end

            StMemRd: begin
                if (!bus_wait) begin
                    d_wren     = 0;
                    d_stb      = 0;
                    rd_wr      = 1;
                    fetch(pc_plus4);
                end
            end

            StMemWr: begin
                if (!bus_wait) begin
                    d_wren     = 0;
                    d_stb      = 0;
                    fetch(pc_plus4);
                end
            end

            default: begin end
        endcase

        if (do_trap) begin
            d_mstatus_mpie = q_mstatus_mie;
            d_mstatus_mie  = 0;

            d_mepc[31:2]   = q_pc[31:2];
            d_mcause_irq   = trap_is_irq;
            d_mcause_code  = trap_code;
            d_mtval        = trap_mtval;

            fetch(q_mtvec);
        end

        d_mip = (d_mip & IRQ_LATCHING) | (irq & IRQ_USED);
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_pc           <= VEC_RESET;
            q_instr        <= 0;
            q_state        <= StFetch;
            q_addr         <= VEC_RESET;
            q_wrdata       <= 0;
            q_bytesel      <= 4'b1111;
            q_wren         <= 0;
            q_stb          <= 1;
            q_mstatus_mie  <= 0;
            q_mstatus_mpie <= 0;
            q_mie          <= 0;
            q_mtvec        <= 0;
            q_mscratch     <= 0;
            q_mepc         <= 0;
            q_mcause_irq   <= 0;
            q_mcause_code  <= 0;
            q_mtval        <= 0;
            q_mip          <= 0;
            q_mcycle       <= 0;

        end else begin
            q_pc           <= d_pc;
            q_instr        <= d_instr;
            q_state        <= d_state;
            q_addr         <= d_addr;
            q_wrdata       <= d_wrdata;
            q_bytesel      <= d_bytesel;
            q_wren         <= d_wren;
            q_stb          <= d_stb;
            q_mstatus_mie  <= d_mstatus_mie;
            q_mstatus_mpie <= d_mstatus_mpie;
            q_mie          <= d_mie;
            q_mtvec        <= d_mtvec;
            q_mscratch     <= d_mscratch;
            q_mepc         <= d_mepc;
            q_mcause_irq   <= d_mcause_irq;
            q_mcause_code  <= d_mcause_code;
            q_mtval        <= d_mtval;
            q_mip          <= d_mip;
            q_mcycle       <= d_mcycle;
        end

endmodule
