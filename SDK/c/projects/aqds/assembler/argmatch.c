#include "argmatch.h"
#include "expr.h"
#include "opcode_info.h"

static const char *regs8_all[]        = {"b", "c", "d", "e", "h", "l", "(hl)", "a"};
static const char *regs8_bcdehlfa[]   = {"b", "c", "d", "e", "h", "l", "f", "a"};
static const char *regs8_ixhl[]       = {"b", "c", "d", "e", "ixh", "ixl", "", "a"};
static const char *regs8_iyhl[]       = {"b", "c", "d", "e", "iyh", "iyl", "", "a"};
static const char *regs_bc_de_hl_sp[] = {"bc", "de", "hl", "sp"};
static const char *regs_bc_de_hl_af[] = {"bc", "de", "hl", "af"};
static const char *regs_bc_de_ix_sp[] = {"bc", "de", "ix", "sp"};
static const char *regs_bc_de_iy_sp[] = {"bc", "de", "iy", "sp"};
static const char *cond_all[]         = {"nz", "z", "nc", "c", "po", "pe", "p", "m"};

uint8_t  cur_outtype = 0;
uint16_t arg_value   = 0;

static bool compare_str(const char *s1, const char *s2) {
    while (to_lower(*s1) == *s2) {
        if (*s1 == 0)
            return true;
        s1++;
        s2++;
    }
    return false;
}

static bool match_imm8_ind(char *arg) {
    if (arg[0] != '(')
        return false;
    cur_p     = (char *)arg + 1;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();
    cur_p++;
    if (arg_value >> 8) {
        error("Value too large!");
    }
    cur_outtype |= OT_8BIT;
    return true;
}

static bool match_imm16_ind(char *arg) {
    if (arg[0] != '(')
        return false;
    cur_p     = (char *)arg + 1;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();

    cur_p++;
    cur_outtype |= OT_16BIT;
    return true;
}

static bool match_c_ind(char *arg) { return compare_str(arg, "(c)"); }
static bool match_bc_ind(char *arg) { return compare_str(arg, "(bc)"); }
static bool match_de_ind(char *arg) { return compare_str(arg, "(de)"); }
static bool match_hl_ind(char *arg) { return compare_str(arg, "(hl)"); }
static bool match_sp_ind(char *arg) { return compare_str(arg, "(sp)"); }
static bool match_20_reg_hl_ind(char *arg) {
    if (compare_str(arg, "(hl)")) {
        cur_opcode |= 6;
        return true;
    }
    return false;
}
static bool match_53_reg_hl_ind(char *arg) {
    if (compare_str(arg, "(hl)")) {
        cur_opcode |= 6 << 3;
        return true;
    }
    return false;
}
static bool match_ix_ind(char *arg) {
    if (compare_str(arg, "(ix)")) {
        cur_outtype |= OT_PREFIX_DD;
        return true;
    }
    return false;
}
static bool match_ix_ind_offs(char *arg) {
    if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'x')
        return false;

    int16_t val = 0;
    if (arg[3] != ')') {
        cur_p = (char *)arg + 3;
        val   = parse_expression(cur_pass == 0);
    }
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();
    if (val < -128 || val > 127)
        range_error();

    d_value = val;
    cur_p++;
    cur_outtype |= OT_PREFIX_DD | OT_D;
    return true;
}
static bool match_iy_ind(char *arg) {
    if (compare_str(arg, "(iy)")) {
        cur_outtype |= OT_PREFIX_FD;
        return true;
    }
    return false;
}
static bool match_iy_ind_offs(char *arg) {
    if (arg[0] != '(' || to_lower(arg[1]) != 'i' || to_lower(arg[2]) != 'y')
        return false;

    int16_t val = 0;
    if (arg[3] != ')') {
        cur_p = (char *)arg + 3;
        val   = parse_expression(cur_pass == 0);
    }
    if (cur_p[0] != ')' && cur_p[1] != 0)
        syntax_error();
    if (val < -128 || val > 127)
        range_error();

    d_value = val;
    cur_p++;
    cur_outtype |= OT_PREFIX_FD | OT_D;
    return true;
}

static bool match_none(char *arg) { return (*arg == 0); }
static bool match_imm_0(char *arg) { return (arg[0] == '0' && arg[1] == 0); }
static bool match_53_imm(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value > 7)
        range_error();
    cur_opcode |= arg_value << 3;
    return true;
}

static bool match_43_imode(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value > 2)
        range_error();
    if (arg_value > 0)
        arg_value += 1;
    cur_opcode |= arg_value << 3;
    return true;
}
static bool match_53_rst(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value < 7) {
        cur_opcode |= arg_value << 3;
    } else if ((arg_value & ~0x38) == 0) {
        cur_opcode |= arg_value;
    } else {
        return false;
    }
    return true;
}
static bool match_imm8(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (arg_value >> 8)
        range_error();
    cur_outtype |= OT_8BIT;
    return true;
}
static bool match_imm16(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    cur_outtype |= OT_16BIT;
    return true;
}
static bool match_rel_addr(char *arg) {
    cur_p     = (char *)arg;
    arg_value = parse_expression(cur_pass == 0);
    if (cur_p[0] != 0)
        syntax_error();

    if (cur_pass == 0) {
        arg_value = 0;
    } else {
        int16_t val = arg_value - (cur_addr + 2);
        if (val < -128 || val > 127)
            range_error();
        arg_value = val & 0xFF;
    }
    cur_outtype |= OT_8BIT;
    return true;
}
static bool match_a(char *arg) { return to_lower(arg[0]) == 'a' && arg[1] == 0; }
static bool match_r(char *arg) { return to_lower(arg[0]) == 'r' && arg[1] == 0; }
static bool match_i(char *arg) { return to_lower(arg[0]) == 'i' && arg[1] == 0; }
static bool match_de(char *arg) { return compare_str(arg, "de"); }
static bool match_hl(char *arg) { return compare_str(arg, "hl"); }
static bool match_sp(char *arg) { return compare_str(arg, "sp"); }
static bool match_af(char *arg) { return compare_str(arg, "af"); }
static bool match_af_alt(char *arg) { return compare_str(arg, "af'"); }
static bool match_20_reg_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i;
            return true;
        }
    }
    return false;
}
static bool match_53_reg_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}
static bool match_53_bcdehlfa(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_bcdehlfa[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}
static bool match_53_cond(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, cond_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}

static bool match_43_cond(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, cond_all[i])) {
            cur_opcode |= i << 3;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_hl_af(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_hl_af[i])) {
            cur_opcode |= i << 4;
            return true;
        }
    }
    return false;
}
static bool match_54_bc_de_hl_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_hl_sp[i])) {
            cur_opcode |= i << 4;
            return true;
        }
    }
    return false;
}
static bool match_ix(char *arg) {
    if (compare_str(arg, "ix")) {
        cur_outtype |= OT_PREFIX_DD;
        return true;
    }
    return false;
}

static bool match_20_ixhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_53_ixhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_20_ixhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_53_ixhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_ixhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_iy_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_iy_sp[i])) {
            cur_opcode |= i << 4;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_iy(char *arg) {
    if (compare_str(arg, "iy")) {
        cur_outtype |= OT_PREFIX_FD;
        return true;
    }
    return false;
}

static bool match_20_iyhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_53_iyhl(char *arg) {
    for (int i = 4; i <= 5; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_20_iyhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_53_iyhl_all(char *arg) {
    for (int i = 0; i < 8; i++) {
        if (compare_str(arg, regs8_iyhl[i])) {
            cur_opcode |= i << 3;
            cur_outtype |= OT_PREFIX_FD;
            return true;
        }
    }
    return false;
}

static bool match_54_bc_de_ix_sp(char *arg) {
    for (int i = 0; i < 4; i++) {
        if (compare_str(arg, regs_bc_de_ix_sp[i])) {
            cur_opcode |= i << 4;
            cur_outtype |= OT_PREFIX_DD;
            return true;
        }
    }
    return false;
}

static bool return_false(char *arg) {
    (void)arg;
    return false;
}

typedef bool match_handler_t(char *arg);

static match_handler_t *ind_match_handlers[] = {
    return_false,        // OD_AT_NONE
    return_false,        // OD_AT_IMM_0
    return_false,        // OD_AT_53_IMM
    return_false,        // OD_AT_43_IMODE
    return_false,        // OD_AT_53_RST
    return_false,        // OD_AT_IMM8
    match_imm8_ind,      // OD_AT_IMM8_IND
    return_false,        // OD_AT_IMM16
    match_imm16_ind,     // OD_AT_IMM16_IND
    return_false,        // OD_AT_REL_ADDR
    return_false,        // OD_AT_A
    return_false,        // OD_AT_R
    return_false,        // OD_AT_I
    return_false,        // OD_AT_DE
    return_false,        // OD_AT_HL
    return_false,        // OD_AT_SP
    return_false,        // OD_AT_AF
    return_false,        // OD_AT_AF_ALT
    match_c_ind,         // OD_AT_C_IND
    match_bc_ind,        // OD_AT_BC_IND
    match_de_ind,        // OD_AT_DE_IND
    match_hl_ind,        // OD_AT_HL_IND
    match_sp_ind,        // OD_AT_SP_IND
    match_20_reg_hl_ind, // OD_AT_20_REG_ALL
    return_false,        // OD_AT_20_BCDEHLA
    match_53_reg_hl_ind, // OD_AT_53_REG_ALL
    return_false,        // OD_AT_53_BCDEHLFA
    return_false,        // OD_AT_53_BCDEHLA
    return_false,        // OD_AT_53_COND
    return_false,        // OD_AT_43_COND
    return_false,        // OD_AT_54_BC_DE_HL_AF
    return_false,        // OD_AT_54_BC_DE_HL_SP
    return_false,        // OD_AT_IX
    match_ix_ind,        // OD_AT_IX_IND
    match_ix_ind_offs,   // OD_AT_IX_IND_OFFS
    return_false,        // OD_AT_20_IXHL
    return_false,        // OD_AT_53_IXHL
    return_false,        // OD_AT_20_IXHL_ALL
    return_false,        // OD_AT_53_IXHL_ALL
    return_false,        // OD_AT_54_BC_DE_IY_SP
    return_false,        // OD_AT_IY
    match_iy_ind,        // OD_AT_IY_IND
    match_iy_ind_offs,   // OD_AT_IY_IND_OFFS
    return_false,        // OD_AT_20_IYHL
    return_false,        // OD_AT_53_IYHL
    return_false,        // OD_AT_20_IYHL_ALL
    return_false,        // OD_AT_53_IYHL_ALL
    return_false,        // OD_AT_54_BC_DE_IX_SP
};

static match_handler_t *dir_match_handlers[] = {
    match_none,           // OD_AT_NONE
    match_imm_0,          // OD_AT_IMM_0
    match_53_imm,         // OD_AT_53_IMM
    match_43_imode,       // OD_AT_43_IMODE
    match_53_rst,         // OD_AT_53_RST
    match_imm8,           // OD_AT_IMM8
    return_false,         // OD_AT_IMM8_IND
    match_imm16,          // OD_AT_IMM16
    return_false,         // OD_AT_IMM16_IND
    match_rel_addr,       // OD_AT_REL_ADDR
    match_a,              // OD_AT_A
    match_r,              // OD_AT_R
    match_i,              // OD_AT_I
    match_de,             // OD_AT_DE
    match_hl,             // OD_AT_HL
    match_sp,             // OD_AT_SP
    match_af,             // OD_AT_AF
    match_af_alt,         // OD_AT_AF_ALT
    return_false,         // OD_AT_C_IND
    return_false,         // OD_AT_BC_IND
    return_false,         // OD_AT_DE_IND
    return_false,         // OD_AT_HL_IND
    return_false,         // OD_AT_SP_IND
    match_20_reg_all,     // OD_AT_20_REG_ALL
    match_20_reg_all,     // OD_AT_20_BCDEHLA
    match_53_reg_all,     // OD_AT_53_REG_ALL
    match_53_bcdehlfa,    // OD_AT_53_BCDEHLFA
    match_53_reg_all,     // OD_AT_53_BCDEHLA
    match_53_cond,        // OD_AT_53_COND
    match_43_cond,        // OD_AT_43_COND
    match_54_bc_de_hl_af, // OD_AT_54_BC_DE_HL_AF
    match_54_bc_de_hl_sp, // OD_AT_54_BC_DE_HL_SP
    match_ix,             // OD_AT_IX
    return_false,         // OD_AT_IX_IND
    return_false,         // OD_AT_IX_IND_OFFS
    match_20_ixhl,        // OD_AT_20_IXHL
    match_53_ixhl,        // OD_AT_53_IXHL
    match_20_ixhl_all,    // OD_AT_20_IXHL_ALL
    match_53_ixhl_all,    // OD_AT_53_IXHL_ALL
    match_54_bc_de_iy_sp, // OD_AT_54_BC_DE_IY_SP
    match_iy,             // OD_AT_IY
    return_false,         // OD_AT_IY_IND
    return_false,         // OD_AT_IY_IND_OFFS
    match_20_iyhl,        // OD_AT_20_IYHL
    match_53_iyhl,        // OD_AT_53_IYHL
    match_20_iyhl_all,    // OD_AT_20_IYHL_ALL
    match_53_iyhl_all,    // OD_AT_53_IYHL_ALL
    match_54_bc_de_ix_sp, // OD_AT_54_BC_DE_IX_SP
};

bool match_argtype(char *arg, uint8_t arg_type) {
    // printf("    - match_argtype(\"%s\", %d)\n", arg, arg_type);
    if (arg[0] == 0 && arg_type != OD_AT_NONE)
        return false;
    if (arg[0] == '(') {
        return ind_match_handlers[arg_type](arg);
    } else {
        return dir_match_handlers[arg_type](arg);
    }
}
