#pragma once

#include "common.h"

void     reloc_init(void);
uint16_t reloc_var_get(const char *ident, unsigned len);
uint16_t reloc_linenr_get(int linenr, uint16_t relocation_bytecode_offset);
void     reloc_linenr_add(int linenr, uint16_t bytecode_offset);
uint16_t reloc_label_get(const char *label, unsigned len, uint16_t relocation_bytecode_offset);
void     reloc_label_add(const char *label, unsigned len, uint16_t bytecode_offset);
void     reloc_process_relocations(void);

extern uint16_t vars_total_size;
