`default_nettype none
`timescale 1 ns / 1 ps

module cpu #(
    parameter VEC_RESET = 32'hFF100000
) (
    input  wire        clk,
    input  wire        reset,

    // Bus interface
    output wire [31:0] bus_addr,
    output wire [31:0] bus_wrdata,
    output wire  [3:0] bus_bytesel,
    output wire        bus_wren,
    output wire  [1:0] bus_cache_op,    // 00:Normal operation, 01:Flush, 10:Clean, 11:Invalidate
    output wire        bus_strobe,
    input  wire        bus_wait,
    input  wire [31:0] bus_rddata,
    input  wire        bus_error,
    input  wire        cpu_tlb_miss,

    output wire        cpu_m_mode,      // Machine mode
    output wire  [5:0] cpu_asid,

    // Interrupt input
    input  wire [15:0] irq);

    localparam [1:0]
        StFetch = 2'd0,
        StExec  = 2'd1,
        StMemRd = 2'd2,
        StMemWr = 2'd3;

    localparam [4:0]
        TrapInstrAddrMisaligned = 5'd0,
        TrapInstrAccessFault    = 5'd1,
        TrapIllegalInstruction  = 5'd2,
        TrapBreakpoint          = 5'd3,
        TrapLoadAddrMisaligned  = 5'd4,
        TrapLoadAccessFault     = 5'd5,
        TrapStoreAddrMisaligned = 5'd6,
        TrapStoreAccessFault    = 5'd7,
        TrapEcallU              = 5'd8,
     // TrapEcallS              = 5'd9,
        TrapEcallM              = 5'd11,
        TrapInstrPageFault      = 5'd12,
        TrapLoadPageFault       = 5'd13,
        TrapStorePageFault      = 5'd15;

    reg [31:0] d_pc,            q_pc;
    reg [31:0] d_instr,         q_instr;
    reg        d_exec_first,    q_exec_first;
    reg  [1:0] d_state,         q_state;
    reg        d_m_mode,        q_m_mode;
    reg [31:0] d_rsv_addr,      q_rsv_addr;
    reg        d_rsv_valid,     q_rsv_valid;

    // Bus interface
    reg [31:0] d_addr,          q_addr;
    reg [31:0] d_wrdata,        q_wrdata;
    reg  [3:0] d_bytesel,       q_bytesel;
    reg        d_wren,          q_wren;
    reg  [1:0] d_cache_op,      q_cache_op;  
    reg        d_stb,           q_stb;
    reg        d_bus_m_mode,    q_bus_m_mode;   // Effective priviledge level on bus

    // CSRs
    reg        d_mstatus_mie,   q_mstatus_mie;  // 0x300 Machine status register:  <3> Machine interrupt enable
    reg        d_mstatus_mpie,  q_mstatus_mpie; // 0x300 Machine status register:  <7> Machine pre-trap interrupt enable
    reg        d_mstatus_mpp,   q_mstatus_mpp;  // 0x300 Machine status register: <12> Machine previous privilege mode
    reg        d_mstatus_mprv,  q_mstatus_mprv; // 0x300 Machine status register: <17> Modify priviledge
    reg [15:0] d_mie,           q_mie;          // 0x304 Machine interrupt-enable register
    reg [31:0] d_mtvec,         q_mtvec;        // 0x305 Machine trap-handler base address
    reg [31:0] d_mscratch,      q_mscratch;     // 0x340 Scratch register for machine trap handlers
    reg [31:0] d_mepc,          q_mepc;         // 0x341 Machine exception program counter
    reg        d_mcause_irq,    q_mcause_irq;   // 0x342 Machine trap cause
    reg  [4:0] d_mcause_code,   q_mcause_code;  // 0x342 Machine trap cause
    reg [31:0] d_mtval,         q_mtval;        // 0x343 Machine bad address or instruction
    reg  [5:0] d_masid,         q_masid;        // 0x7C0 Address space identifier
    reg [63:0] d_mcycle,        q_mcycle;       // 0xB00/0xB80 Machine cycle counter

    assign bus_addr     = q_addr;
    assign bus_wrdata   = q_wrdata;
    assign bus_bytesel  = q_bytesel;
    assign bus_wren     = q_wren;
    assign bus_cache_op = q_cache_op;
    assign bus_strobe   = q_stb;

    assign cpu_m_mode   = q_bus_m_mode;
    assign cpu_asid     = q_masid;

    //////////////////////////////////////////////////////////////////////////
    // Instruction decoding
    //////////////////////////////////////////////////////////////////////////
    wire  [6:0] opcode  = q_instr[6:0];
    wire  [2:0] funct3  = q_instr[14:12];
    wire  [6:0] funct7  = q_instr[31:25];
    wire  [4:0] rs1_idx = q_instr[19:15];
    wire  [4:0] rs2_idx = q_instr[24:20];
    wire  [4:0] rd_idx  = q_instr[11:7];
    wire [31:0] imm_i   = {{21{q_instr[31]}}, q_instr[30:20]};
    wire [31:0] imm_s   = {{21{q_instr[31]}}, q_instr[30:25], q_instr[11:7]};
    wire [31:0] imm_b   = {{20{q_instr[31]}}, q_instr[7], q_instr[30:25], q_instr[11:8], 1'b0};
    wire [31:0] imm_u   = {q_instr[31:12], 12'b0};
    wire [31:0] imm_j   = {{12{q_instr[31]}}, q_instr[19:12], q_instr[20], q_instr[30:21], 1'b0};
    wire [11:0] csr     = q_instr[31:20];

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
    wire is_atomic  = (opcode == 7'b0101111 && funct3 == 3'b010); // Atomic instructions (funct7 will select function)
    wire is_zaamo   = is_atomic && !funct7[3];
    wire is_lrw     = is_atomic && funct7[6:2] == 5'b00010;
    wire is_scw     = is_atomic && funct7[6:2] == 5'b00011;
    wire is_fence   = (funct3 == 3'b000 && opcode == 7'b0001111);                   // FENCE/PAUSE
    wire is_cbo     = (funct3 == 3'b010 && rd_idx == 5'd0 && opcode == 7'b0001111); // CBO

    wire is_ecall  = is_system && q_instr[31:7] == 25'b0000000_00000_00000_000_00000;
    wire is_ebreak = is_system && q_instr[31:7] == 25'b0000000_00001_00000_000_00000;
    wire is_mret   = is_system && q_instr[31:7] == 25'b0011000_00010_00000_000_00000;
    wire is_csr    = is_system && funct3[1:0] != 2'b00;

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
        is_atomic  ||
        ( q_m_mode && (is_system || is_cbo)) ||
        (!q_m_mode && (is_ecall || is_ebreak));   // In user mode only allow ecall/ebreak system instructions

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
    wire        div_done;
    wire [31:0] div_quotient;
    wire [31:0] div_remainder;
    wire        div_busy;   // unused
    reg         div_start;

    div div(
        .clk(clk),
        .reset(reset),

        .operand_l(rs1_data),
        .operand_r(rs2_data),
        .is_signed(!funct3[0] && is_div_rem),
        .start(div_start),

        .busy(div_busy),
        .done(div_done),
        .quotient(div_quotient),
        .remainder(div_remainder)
    );

    // Operation selection
    reg [31:0] alu_result;
    always @* begin
        if (is_mul_div) begin
            case (funct3)
                3'b000: alu_result = mult_result[31:0];     // MUL
                3'b001: alu_result = mult_result[63:32];    // MULH
                3'b010: alu_result = mult_result[63:32];    // MULHSU
                3'b011: alu_result = mult_result[63:32];    // MULHU
                3'b100: alu_result = div_quotient;          // DIV
                3'b101: alu_result = div_quotient;          // DIVU
                3'b110: alu_result = div_remainder;         // REM
                3'b111: alu_result = div_remainder;         // REMU
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
        if (funct3[1])          // LW
            load_data = bus_rddata;
        else if (funct3[0])     // LH/LHU
            load_data = funct3[2] ? {16'b0, lh_data} : {{16{lh_data[15]}}, lh_data};
        else                    // LB/LBU
            load_data = funct3[2] ? {24'b0, lb_data} : {{24{lb_data[7]}}, lb_data};
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

    // Custom CSR
    wire is_masid    = (csr == 12'h7C0);    // Address space identifier

    // Machine Counter/Timers
    wire is_mcycle   = (csr == 12'hB00);    // Machine cycle counter
    wire is_mcycleh  = (csr == 12'hB80);    // Upper 32 bits of mcycle

    // mstatus register
    reg [31:0] mstatus;
    always @* begin
        mstatus     = 0;
        mstatus[17] = q_mstatus_mprv;
        mstatus[12] = q_mstatus_mpp;
        mstatus[11] = q_mstatus_mpp;
        mstatus[7]  = q_mstatus_mpie;
        mstatus[3]  = q_mstatus_mie;
    end

    reg [31:0] csr_rdata;
    always @* begin
        csr_rdata = 32'h00000000;

        if (is_mstatus)   csr_rdata = mstatus;
        if (is_mie)       csr_rdata = {q_mie, 16'b0};
        if (is_mtvec)     csr_rdata = q_mtvec;
        if (is_mscratch)  csr_rdata = q_mscratch;
        if (is_mepc)      csr_rdata = {q_mepc[31:2], 2'b0};
        if (is_mcause)    csr_rdata = {q_mcause_irq, 26'b0, q_mcause_code};
        if (is_mtval)     csr_rdata = q_mtval;
        if (is_mip)       csr_rdata = {irq, 16'b0};
        if (is_masid)     csr_rdata = {26'b0, q_masid};
        if (is_mcycle)    csr_rdata = q_mcycle[31:0];
        if (is_mcycleh)   csr_rdata = q_mcycle[63:32];
    end

    wire [31:0] csr_operand = funct3[2] ? {27'd0, rs1_idx} : rs1_data;
    reg  [31:0] csr_wdata;
    always @*
        if      (funct3[1:0] == 2'b10)  csr_wdata = csr_rdata |  csr_operand;   // CSRRS(I) set-bits
        else if (funct3[1:0] == 2'b11)  csr_wdata = csr_rdata & ~csr_operand;   // CSRRC(I)clear-bits
        else                            csr_wdata =              csr_operand;   // CSRRW(I) assign

    // For both CSRRS and CSRRC, if rs1=x0, then the instruction will not write to the CSR at all.
    wire csr_write = is_csr && !(funct3[2:1] == 2'b01 && rs1_idx == 5'd0);

    //////////////////////////////////////////////////////////////////////////
    // Interrupts
    //////////////////////////////////////////////////////////////////////////
    reg [15:0] q_irq;
    always @(posedge clk) q_irq <= irq;

    reg [3:0] irq_code;
    always @* begin
        irq_code = 0;
        if (irq[15]) irq_code = 4'd15;
        if (irq[14]) irq_code = 4'd14;
        if (irq[13]) irq_code = 4'd13;
        if (irq[12]) irq_code = 4'd12;
        if (irq[11]) irq_code = 4'd11;
        if (irq[10]) irq_code = 4'd10;
        if (irq[ 9]) irq_code = 4'd9;
        if (irq[ 8]) irq_code = 4'd8;
        if (irq[ 7]) irq_code = 4'd7;
        if (irq[ 6]) irq_code = 4'd6;
        if (irq[ 5]) irq_code = 4'd5;
        if (irq[ 4]) irq_code = 4'd4;
        if (irq[ 3]) irq_code = 4'd3;
        if (irq[ 2]) irq_code = 4'd2;
        if (irq[ 1]) irq_code = 4'd1;
        if (irq[ 0]) irq_code = 4'd0;
    end

    wire irq_pending = (q_irq & q_mie) != 0;

    //////////////////////////////////////////////////////////////////////////
    // State machine
    //////////////////////////////////////////////////////////////////////////
    wire [31:0] pc_plus4    = q_pc + 32'd4;
    wire [31:0] pc_plus_imm = q_pc + (is_jal ? imm_j : (is_auipc ? imm_u : imm_b));

    always @* begin
        rd_data = 32'b0;

        if (is_lui)
            rd_data = imm_u;
        else if (is_auipc)
            rd_data = pc_plus_imm;
        else if (is_jal || is_jalr)
            rd_data = pc_plus4;
        else if (is_load || is_zaamo || is_lrw)
            rd_data = load_data;
        else if (is_alu_imm || is_alu_reg)
            rd_data = alu_result;
        else if (is_csr)
            rd_data = csr_rdata;
        else if (is_scw)
            rd_data = (q_rsv_valid && q_rsv_addr == rs1_data) ? 32'b0 : 32'b1;
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
            if (newpc[1:0] != 0)
                trap(0, TrapInstrAddrMisaligned, newpc);

            d_pc         = {newpc[31:2], 2'b00};

            d_addr       = {newpc[31:2], 2'b00};
            d_bytesel    = 4'b1111;
            d_wren       = 0;
            d_cache_op   = 0;
            d_stb        = 1;
            d_bus_m_mode = d_m_mode;
        end
    endtask

    always @* begin
        d_pc           = q_pc;
        d_instr        = q_instr;
        d_exec_first   = 0;
        d_state        = q_state;
        d_m_mode       = q_m_mode;
        d_rsv_addr     = q_rsv_addr;
        d_rsv_valid    = q_rsv_valid;

        d_addr         = q_addr;
        d_wrdata       = q_wrdata;
        d_bytesel      = q_bytesel;
        d_wren         = q_wren;
        d_stb          = q_stb;
        d_bus_m_mode   = q_bus_m_mode;
        d_cache_op     = q_cache_op;

        d_mstatus_mie  = q_mstatus_mie;
        d_mstatus_mpie = q_mstatus_mpie;
        d_mstatus_mpp  = q_mstatus_mpp;
        d_mstatus_mprv = q_mstatus_mprv;
        d_mie          = q_mie;
        d_mtvec        = q_mtvec;
        d_mscratch     = q_mscratch;
        d_mepc         = q_mepc;
        d_mcause_irq   = q_mcause_irq;
        d_mcause_code  = q_mcause_code;
        d_mtval        = q_mtval;
        d_masid        = q_masid;
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
                    d_cache_op = 0;
                    d_stb      = 0;

                    if (irq_pending && q_mstatus_mie) begin
                        trap(1, {1'b1, irq_code}, 0);

                    end else if (bus_error) begin
                        trap(0, cpu_tlb_miss ? TrapInstrPageFault : TrapInstrAccessFault, q_addr);

                    end else begin
                        d_state      = StExec;
                        d_instr      = bus_rddata;
                        d_exec_first = 1;
                    end
                end
            end

            StExec: begin
                if (!is_valid_instruction) begin
                    trap(0, TrapIllegalInstruction, 0);

                end else if (is_ebreak || is_ecall) begin
                    if (is_ebreak)
                        trap(0, TrapBreakpoint, 0);
                    else if (is_ecall)
                        trap(0, q_m_mode ? TrapEcallM : TrapEcallU, 0);

                end else if (is_mret) begin
                    d_mstatus_mie  = q_mstatus_mpie;
                    d_mstatus_mpie = 1;
                    if (!q_mstatus_mpp)
                        d_mstatus_mprv = 0;
                    d_m_mode       = q_mstatus_mpp;
                    d_mstatus_mpp  = 0;

                    fetch(q_mepc);

                end else if (is_div_rem && (q_exec_first || div_busy)) begin
                    div_start = q_exec_first;
                    // Wait for division to be done

                end else if (is_load || is_store) begin
                    // Check load/store alignment
                    if (funct3[1] ? (load_store_addr[1:0] != 0) : (funct3[0] ? (load_store_addr[0] != 0) : 0))
                        trap(0, is_load ? TrapLoadAddrMisaligned : TrapStoreAddrMisaligned, load_store_addr);

                    d_state      = is_load ? StMemRd : StMemWr;
                    d_addr       = load_store_addr;
                    d_wren       = is_store;
                    d_cache_op   = 0;
                    d_stb        = 1;
                    d_bus_m_mode = (q_m_mode && q_mstatus_mprv) ? q_mstatus_mpp : q_m_mode;

                    if (funct3[1])          // SW
                        d_wrdata = rs2_data;
                    else if (funct3[0])     // SH
                        d_wrdata = {rs2_data[15:0], rs2_data[15:0]};
                    else                    // SB
                        d_wrdata = {rs2_data[7:0], rs2_data[7:0], rs2_data[7:0], rs2_data[7:0]};

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

                end else if (is_cbo) begin
                    d_state      = StMemRd;
                    d_addr       = rs1_data;
                    d_wren       = 0;
                    d_cache_op   = ~q_instr[21:20];
                    d_stb        = 1;
                    d_bus_m_mode = (q_m_mode && q_mstatus_mprv) ? q_mstatus_mpp : q_m_mode;

                end else if (is_zaamo) begin
                    d_state      = StMemRd;
                    d_addr       = rs1_data;
                    d_wren       = 0;
                    d_cache_op   = 0;
                    d_stb        = 1;
                    d_bus_m_mode = (q_m_mode && q_mstatus_mprv) ? q_mstatus_mpp : q_m_mode;

                end else if (is_lrw) begin
                    if (rs1_data[1:0] != 0)
                        trap(0, TrapLoadAddrMisaligned, rs1_data);

                    // LR.W
                    d_state       = StMemRd;
                    d_addr        = rs1_data;
                    d_rsv_addr    = rs1_data;
                    d_rsv_valid   = 1;
                    d_cache_op    = 0;
                    d_stb         = 1;
                    d_bus_m_mode  = (q_m_mode && q_mstatus_mprv) ? q_mstatus_mpp : q_m_mode;

                end else if (is_scw) begin
                    if (rs1_data[1:0] != 0)
                        trap(0, TrapStoreAddrMisaligned, rs1_data);

                    d_rsv_valid = 0;
                    rd_wr       = 1;

                    if (q_rsv_valid && q_rsv_addr == rs1_data) begin
                        d_state      = StMemWr;
                        d_addr       = rs1_data;
                        d_wren       = 1;
                        d_cache_op   = 0;
                        d_stb        = 1;
                        d_bus_m_mode = (q_m_mode && q_mstatus_mprv) ? q_mstatus_mpp : q_m_mode;
                        d_wrdata     = rs2_data;
                        d_bytesel    = 4'b1111;
                    end else begin
                        fetch(pc_plus4);
                    end

                end else begin
                    if (is_lui || is_auipc || is_jal || is_jalr || is_alu_imm || is_alu_reg || is_system)
                        rd_wr = 1;

                    // CSR write handling
                    if (csr_write) begin
                        if (is_mstatus) begin
                            d_mstatus_mprv = csr_wdata[17];
                            d_mstatus_mpp  = csr_wdata[12];
                            d_mstatus_mpie = csr_wdata[7];
                            d_mstatus_mie  = csr_wdata[3];
                        end

                        if (is_mie)      d_mie           = csr_wdata[31:16];
                        if (is_mtvec)    d_mtvec         = {csr_wdata[31:2], 2'b0};
                        if (is_mscratch) d_mscratch      = csr_wdata;
                        if (is_mepc)     d_mepc[31:2]    = csr_wdata[31:2];

                        if (is_mcause) begin
                            d_mcause_irq  = csr_wdata[31];
                            d_mcause_code = csr_wdata[4:0];
                        end

                        if (is_mtval)    d_mtval         = csr_wdata;
                        if (is_masid)    d_masid         = csr_wdata[5:0];
                        if (is_mcycle)   d_mcycle[31:0]  = csr_wdata;
                        if (is_mcycleh)  d_mcycle[63:32] = csr_wdata;
                    end

                    if (is_jal || do_branch) fetch(pc_plus_imm);
                    else if (is_jalr)        fetch(alu_add);
                    else                     fetch(pc_plus4);
                end
            end

            StMemRd: begin
                if (!bus_wait) begin
                    d_wren     = 0;
                    d_cache_op = 0;
                    d_stb      = 0;

                    if (bus_error) begin
                        trap(0, cpu_tlb_miss ? TrapLoadPageFault : TrapLoadAccessFault, q_addr);
                    end else begin
                        rd_wr = 1;

                        if (is_zaamo) begin
                            // Calculate value and write back to memory
                            d_wren     = 1;
                            d_cache_op = 0;
                            d_stb      = 1;
                            d_bytesel  = 4'b1111;
                            d_state    = StMemWr;

                            case (funct7[6:2])
                                5'b00000: d_wrdata = rs2_data + bus_rddata;                             // AMOADD.W
                                5'b00001: d_wrdata = rs2_data;                                          // AMOSWAP.W
                                5'b00100: d_wrdata = rs2_data ^ bus_rddata;                             // AMOXOR.W
                                5'b01000: d_wrdata = rs2_data | bus_rddata;                             // AMOOR.W
                                5'b01100: d_wrdata = rs2_data & bus_rddata;                             // AMOAND.W
                                5'b10000: d_wrdata = rs2_data_s < bus_rddata_s ? rs2_data : bus_rddata; // AMOMIN.W
                                5'b10100: d_wrdata = rs2_data_s > bus_rddata_s ? rs2_data : bus_rddata; // AMOMAX.W
                                5'b11000: d_wrdata = rs2_data   < bus_rddata   ? rs2_data : bus_rddata; // AMOMINU.W
                                5'b11100: d_wrdata = rs2_data   > bus_rddata   ? rs2_data : bus_rddata; // AMOMAXU.W
                                default:  trap(0, TrapIllegalInstruction, 0);                           // Invalid instruction
                            endcase

                        end else begin
                            fetch(pc_plus4);
                        end
                    end
                end
            end

            StMemWr: begin
                if (!bus_wait) begin
                    d_wren     = 0;
                    d_cache_op = 0;
                    d_stb      = 0;

                    if (bus_error) begin
                        trap(0, cpu_tlb_miss ? TrapStorePageFault : TrapStoreAccessFault, q_addr);
                    end else begin
                        fetch(pc_plus4);
                    end
                end
            end

            default: begin end
        endcase

        if (do_trap) begin
            d_m_mode       = 1;
            d_mstatus_mpp  = q_m_mode;
            d_mstatus_mpie = q_mstatus_mie;
            d_mstatus_mie  = 0;
            d_rsv_valid    = 0;

            d_mepc[31:2]   = q_pc[31:2];
            d_mcause_irq   = trap_is_irq;
            d_mcause_code  = trap_code;
            d_mtval        = trap_mtval;

            fetch(q_mtvec);
        end
    end

    always @(posedge clk or posedge reset)
        if (reset) begin
            q_pc           <= VEC_RESET;
            q_instr        <= 0;
            q_exec_first   <= 0;
            q_state        <= StFetch;
            q_m_mode       <= 1;     // Start in machine mode
            q_rsv_addr     <= 0;
            q_rsv_valid    <= 0;

            q_addr         <= VEC_RESET;
            q_wrdata       <= 0;
            q_bytesel      <= 4'b1111;
            q_wren         <= 0;
            q_cache_op     <= 0;
            q_stb          <= 1;
            q_bus_m_mode   <= 1;

            q_mstatus_mie  <= 0;
            q_mstatus_mpie <= 0;
            q_mstatus_mpp  <= 1;
            q_mstatus_mprv <= 0;
            q_mie          <= 0;
            q_mtvec        <= 0;
            q_mscratch     <= 0;
            q_mepc         <= 0;
            q_mcause_irq   <= 0;
            q_mcause_code  <= 0;
            q_mtval        <= 0;
            q_masid        <= 0;
            q_mcycle       <= 0;

        end else begin
            q_pc           <= d_pc;
            q_instr        <= d_instr;
            q_exec_first   <= d_exec_first;
            q_state        <= d_state;
            q_m_mode       <= d_m_mode;
            q_rsv_addr     <= d_rsv_addr;
            q_rsv_valid    <= d_rsv_valid;

            q_addr         <= d_addr;
            q_wrdata       <= d_wrdata;
            q_bytesel      <= d_bytesel;
            q_wren         <= d_wren;
            q_cache_op     <= d_cache_op;
            q_stb          <= d_stb;
            q_bus_m_mode   <= d_bus_m_mode;

            q_mstatus_mie  <= d_mstatus_mie;
            q_mstatus_mpie <= d_mstatus_mpie;
            q_mstatus_mpp  <= d_mstatus_mpp;
            q_mstatus_mprv <= d_mstatus_mprv;
            q_mie          <= d_mie;
            q_mtvec        <= d_mtvec;
            q_mscratch     <= d_mscratch;
            q_mepc         <= d_mepc;
            q_mcause_irq   <= d_mcause_irq;
            q_mcause_code  <= d_mcause_code;
            q_mtval        <= d_mtval;
            q_masid        <= d_masid;
            q_mcycle       <= d_mcycle;
        end

endmodule
