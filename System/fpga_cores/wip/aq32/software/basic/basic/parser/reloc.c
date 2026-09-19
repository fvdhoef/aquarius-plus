#include "reloc.h"
#include "basic.h"
#include "tokenizer.h"
#include "bytecode/bytecode.h"
#include "common/buffers.h"

static uint8_t *ptr_alloc;
static uint8_t *ptr_alloc_end;
static uint8_t *ptr_alloc_buf_end;

//
// list_var_offsets layout:
//   u16  next
//   u16  var_offset
//   u8   name_len
//   u8[] name with type indicator
//
// list_label_offsets layout:
//   u16  next
//   u16  bytecode_offset
//   u8   name_len
//   u8[] name with type indicator
//
// list_line_offsets layout:
//   u16  next
//   u16  bytecode_offset
//   u16  linenr
//
// list_relocations layout:
//   u16  next
//   u16  bytecode_offset
//   u16  offset (in ptr_alloc area)
//
static uint16_t list_var_offsets    = 0;
static uint16_t list_label_offsets  = 0;
static uint16_t list_linenr_offsets = 0;
static uint16_t list_relocations    = 0;
uint16_t        vars_total_size;

static uint16_t alloc_record(unsigned size) {
    if (ptr_alloc_end + size > ptr_alloc_buf_end)
        _basic_error(ERR_OUT_OF_MEM);

    uint8_t *result = ptr_alloc_end;
    ptr_alloc_end += size;
    return result - ptr_alloc;
}

void reloc_init(void) {
    size_t workspace_size = 0x10000;

    ptr_alloc = buf_malloc(workspace_size);
    if (!ptr_alloc)
        _basic_error(ERR_OUT_OF_MEM);

    ptr_alloc_end     = ptr_alloc + 1; // Reserve offset 0 for list termination
    ptr_alloc_buf_end = ptr_alloc + workspace_size;

    vars_total_size = 0;

    list_var_offsets    = 0;
    list_label_offsets  = 0;
    list_linenr_offsets = 0;
    list_relocations    = 0;
}

static void _add_relocation(uint16_t bytecode_offset, uint16_t rec_offset) {
    unsigned rec_sz   = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t);
    uint16_t rec_next = list_relocations;

    uint16_t o = alloc_record(rec_sz);
    write_u16(&ptr_alloc[o], rec_next);
    write_u16(&ptr_alloc[o + 2], bytecode_offset);
    write_u16(&ptr_alloc[o + 4], rec_offset);

    list_relocations = o;
}

void reloc_process_relocations(void) {
    uint16_t o = list_relocations;
    while (o != 0) {
        uint16_t rec_next            = read_u16(&ptr_alloc[o]);
        uint16_t rec_bytecode_offset = read_u16(&ptr_alloc[o + 2]);
        uint16_t rec_offset          = read_u16(&ptr_alloc[o + 4]);

        uint16_t target_offset = read_u16(&ptr_alloc[rec_offset]);
        if (target_offset == 0xFFFF) {
            err_line = bytecode_get_line_for_offset(buf_bytecode, buf_bytecode_end - buf_bytecode, rec_bytecode_offset);
            _basic_error(ERR_LABEL_NOT_DEFINED);
        }
        buf_bytecode_patch_u16(rec_bytecode_offset, target_offset);

        o = rec_next;
    }
}

static unsigned get_data_size(uint8_t type) {
    switch (type) {
        case '%': return sizeof(int16_t);
        case '&': return sizeof(int32_t);
        case '!': return sizeof(float);
        case '#': return sizeof(double);
        case '$': return sizeof(char *);
        case '(': return sizeof(void *);
        default: _basic_error(ERR_INTERNAL_ERROR); return 0;
    }
}

uint16_t reloc_var_get(const char *ident, unsigned len) {
    // Search for existing variable
    uint16_t o = list_var_offsets;
    while (o != 0) {
        uint16_t rec_next  = read_u16(&ptr_alloc[o]);
        uint8_t  ident_len = ptr_alloc[o + 4];
        if (ident_len == len && memcmp(&ptr_alloc[o + 5], ident, len) == 0) {
            uint16_t var_offset = read_u16(&ptr_alloc[o + 2]);
            return var_offset;
        }
        o = rec_next;
    }

    // Not found, allocate new variable
    unsigned data_sz = get_data_size(ident[len - 1]);
    if (vars_total_size + data_sz > 0x10000)
        _basic_error(ERR_OUT_OF_MEM);

    unsigned rec_sz     = sizeof(uint16_t) + sizeof(uint16_t) + 1 + len;
    uint16_t rec_next   = list_var_offsets;
    uint16_t var_offset = vars_total_size;
    vars_total_size += data_sz;

    o = alloc_record(rec_sz);
    write_u16(&ptr_alloc[o], rec_next);
    write_u16(&ptr_alloc[o + 2], var_offset);
    ptr_alloc[o + 4] = len;
    memcpy(&ptr_alloc[o + 5], ident, len);

    list_var_offsets = o;

    return var_offset;
}

static uint16_t _linenr_search(int linenr) {
    // Search for existing line number
    uint16_t o = list_linenr_offsets;
    while (o != 0) {
        uint16_t rec_next   = read_u16(&ptr_alloc[o]);
        uint16_t rec_linenr = read_u16(&ptr_alloc[o + 4]);
        if (rec_linenr == linenr) {
            return o + 2;
        }
        o = rec_next;
    }
    return 0;
}

static uint16_t _add_linenr(uint16_t linenr, uint16_t bytecode_offset) {
    unsigned rec_sz   = sizeof(uint16_t) + sizeof(uint16_t) + sizeof(uint16_t);
    uint16_t rec_next = list_linenr_offsets;

    uint16_t o = alloc_record(rec_sz);
    write_u16(&ptr_alloc[o], rec_next);
    write_u16(&ptr_alloc[o + 2], bytecode_offset);
    write_u16(&ptr_alloc[o + 4], linenr);

    list_linenr_offsets = o;

    return o + 2;
}

uint16_t reloc_linenr_get(int linenr, uint16_t relocation_bytecode_offset) {
    // Search for existing line number
    uint16_t o = _linenr_search(linenr);
    while (o != 0) {
        uint16_t result = read_u16(ptr_alloc + o);
        if (result == 0xFFFF)
            _add_relocation(relocation_bytecode_offset, o);
        return result;
    }

    // Line number does not exist, create unknown line number and relocation entry
    _add_relocation(relocation_bytecode_offset, _add_linenr(linenr, 0xFFFF));
    return 0xFFFF;
}

void reloc_linenr_add(int linenr, uint16_t bytecode_offset) {
    // Search for existing line number
    uint16_t o = _linenr_search(linenr);
    if (o != 0) {
        uint16_t result = read_u16(ptr_alloc + o);
        if (result == 0xFFFF) {
            // Update existing record
            ptr_alloc[o]     = bytecode_offset & 0xFF;
            ptr_alloc[o + 1] = (bytecode_offset >> 8) & 0xFF;
        } else {
            _basic_error(ERR_DUPLICATE_LABEL);
        }
        return;
    }

    // Add line number
    _add_linenr(linenr, bytecode_offset);
}

static uint16_t _label_search(const char *label, unsigned len) {
    // Search for existing label
    uint16_t o = list_label_offsets;
    while (o != 0) {
        uint16_t rec_next     = read_u16(&ptr_alloc[o]);
        uint8_t  rec_name_len = ptr_alloc[o + 4];
        if (rec_name_len == len && memcmp(&ptr_alloc[o + 5], label, len) == 0) {
            return o + 2;
        }
        o = rec_next;
    }
    return 0;
}

static uint16_t _add_label(const char *label, unsigned len, uint16_t bytecode_offset) {
    unsigned rec_sz   = sizeof(uint16_t) + sizeof(uint16_t) + 1 + len;
    uint16_t rec_next = list_label_offsets;

    uint16_t o = alloc_record(rec_sz);
    write_u16(&ptr_alloc[o], rec_next);
    write_u16(&ptr_alloc[o + 2], bytecode_offset);
    ptr_alloc[o + 4] = len;
    memcpy(&ptr_alloc[o + 5], label, len);

    list_label_offsets = o;

    return o + 2;
}

uint16_t reloc_label_get(const char *label, unsigned len, uint16_t relocation_bytecode_offset) {
    // Search for existing label
    uint16_t o = _label_search(label, len);
    if (o != 0) {
        uint16_t result = read_u16(ptr_alloc + o);
        if (result == 0xFFFF)
            _add_relocation(relocation_bytecode_offset, o);
        return result;
    }

    // Label does not exist, create unknown label and relocation entry
    _add_relocation(relocation_bytecode_offset, _add_label(label, len, 0xFFFF));
    return 0xFFFF;
}

void reloc_label_add(const char *label, unsigned len, uint16_t bytecode_offset) {
    // Search for existing label
    uint16_t o = _label_search(label, len);
    if (o != 0) {
        uint16_t result = read_u16(ptr_alloc + o);
        if (result == 0xFFFF) {
            // Update existing record
            ptr_alloc[o]     = bytecode_offset & 0xFF;
            ptr_alloc[o + 1] = (bytecode_offset >> 8) & 0xFF;
        } else {
            _basic_error(ERR_DUPLICATE_LABEL);
        }
        return;
    }

    // Add label
    _add_label(label, len, bytecode_offset);
}
