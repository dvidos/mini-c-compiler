#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "asm_line.h"
#include "../run_info.h"
#include "../err_handler.h"
#include "../utils/all.h"


asm_line *new_asm_line_empty(mempool *mp) {
    asm_line *line = mpalloc(mp, asm_line);
    line->type = ALT_EMPTY;
    return line;
}

asm_line *new_asm_line_directive_section(mempool *mp, str *section_name) {
    asm_line *l = new_asm_line_empty(mp);
    l->type = ALT_SECTION;
    l->per_type.named_definition = mpalloc(mp, asm_directive);
    l->per_type.named_definition->name = section_name;
    return l;
}

asm_line *new_asm_line_directive_extern(mempool *mp, str *symbol_name) {
    asm_line *l = new_asm_line_empty(mp);
    l->type = ALT_EXTERN;
    l->per_type.named_definition = mpalloc(mp, asm_directive);
    l->per_type.named_definition->name = symbol_name;
    return l;
}

asm_line *new_asm_line_directive_global(mempool *mp, str *symbol_name) {
    asm_line *l = new_asm_line_empty(mp);
    l->type = ALT_GLOBAL;
    l->per_type.named_definition = mpalloc(mp, asm_directive);
    l->per_type.named_definition->name = symbol_name;
    return l;
}

asm_line *new_asm_line_data_definition(mempool *mp, str *symbol_name, data_size unit_size, size_t units, bin *initial_value) {
    int unit_bytes = 0;
    switch (unit_size) {
        case DATA_BYTE:  unit_bytes = 1; break;
        case DATA_WORD:  unit_bytes = 2; break;
        case DATA_DWORD: unit_bytes = 4; break;
        case DATA_QWORD: unit_bytes = 8; break;
    }
    
    asm_data_definition *dd = mpalloc(mp, asm_data_definition);
    dd = mpalloc(mp, asm_data_definition);
    dd->name = symbol_name;
    dd->unit_size = unit_size;
    dd->length_units = units;
    dd->length_bytes = units * unit_bytes;
    dd->initial_value = initial_value;

    asm_line *l = new_asm_line_empty(mp);
    l->type = ALT_DATA;
    l->per_type.data_definition = dd;
    return l;
}

asm_line *new_asm_line_instruction(mempool *mp, instr_code op) {
    asm_instruction *i = mpalloc(mp, asm_instruction);
    i->operation = op;
    i->direction_regmem_to_regimm = true;
    i->operands_data_size = DATA_QWORD;

    asm_line *l = new_asm_line_empty(mp);
    l->type = ALT_INSTRUCTION;
    l->per_type.instruction = i;
    return l;
}

asm_line *new_asm_line_instruction_with_operand(mempool *mp, instr_code op, asm_operand *target) {
    asm_line *l = new_asm_line_instruction(mp, op);
    asm_instruction *i = l->per_type.instruction;
    
    // operand1 should be either register or memory
    if (target->type == OT_REGISTER) {
        i->regmem_operand.is_register = true;
        i->regmem_operand.per_type.reg = target->reg;

    } else if (target->type == OT_MEM_POINTED_BY_REG) {
        i->regmem_operand.is_memory_by_reg = true;
        i->regmem_operand.per_type.mem.pointer_reg = target->reg;
        i->regmem_operand.per_type.mem.displacement = target->offset;

    } else if (target->type == OT_MEM_OF_SYMBOL) {
        i->regmem_operand.is_mem_addr_by_symbol = true;
        i->regmem_operand.per_type.mem.displacement_symbol_name = strdup(target->symbol_name);

    } else if (target->type == OT_IMMEDIATE) {
        // immediates go to operand2, and usually coded without the ModRM byte
        i->regimm_operand.is_immediate = true;
        i->regimm_operand.per_type.immediate = target->immediate;
    }

    return l;
}

asm_line *new_asm_line_instruction_with_operands(mempool *mp, instr_code op, asm_operand *target, asm_operand *source) {
    // operations memory to memory, or any to immediate, are not supported
    if ((source->type == OT_MEM_OF_SYMBOL || source->type == OT_MEM_POINTED_BY_REG) &&
        (target->type == OT_MEM_OF_SYMBOL || target->type == OT_MEM_POINTED_BY_REG)) {
        error("mem-to-mem operations cannot be codified, use of register needed");
        return NULL;
    }
    if (target->type == OT_IMMEDIATE) {
        error("operations towards immediates cannot be codified, use of register needed");
        return NULL;
    }
    
    // target   source   operand1   operand2    direction
    // ----------------------------------------------------
    // reg      reg      reg (t)    reg (s)     op2 to op1
    // reg      imm      reg (t)    imm (s)     op2 to op1
    // reg      mem      mem (s)    reg (t)     op1 to op2 <-- only case op1 to op2
    // mem      reg      mem (t)    reg (s)     op2 to op1
    // mem      imm      mem (t)    imm (s)     op2 to op1

    asm_operand *operand1;
    asm_operand *operand2;
    bool dir_1_to_2;
    if (target->type == OT_REGISTER && 
        (source->type == OT_MEM_OF_SYMBOL || source->type == OT_MEM_POINTED_BY_REG)) {
        operand1 = source;
        operand2 = target;
        dir_1_to_2 = true;
    } else {
        operand1 = target;
        operand2 = source;
        dir_1_to_2 = false;
    }

    asm_line *l = new_asm_line_instruction(mp, op);
    asm_instruction *i = l->per_type.instruction;
    
    // operand1 should be either register or memory
    if (operand1->type == OT_REGISTER) {
        i->regmem_operand.is_register = true;
        i->regmem_operand.per_type.reg = operand1->reg;
    } else if (operand1->type == OT_MEM_POINTED_BY_REG) {
        i->regmem_operand.is_memory_by_reg = true;
        i->regmem_operand.per_type.mem.pointer_reg = operand1->reg;
        i->regmem_operand.per_type.mem.displacement = operand1->offset;
    } else if (operand1->type == OT_MEM_OF_SYMBOL) {
        i->regmem_operand.is_mem_addr_by_symbol = true;
        i->regmem_operand.per_type.mem.displacement_symbol_name = strdup(operand1->symbol_name);
    } else {
        error("possible bug, expected register or memory for operand 1");
        return NULL;
    }

    // operand2 should be either register or immediate
    if (operand2->type == OT_REGISTER) {
        i->regimm_operand.is_register = true;
        i->regimm_operand.per_type.reg = operand2->reg;
    } else if (operand2->type == OT_IMMEDIATE) {
        i->regimm_operand.is_immediate = true;
        i->regimm_operand.per_type.immediate = operand2->immediate;
    } else {
        error("possible bug, expected register or immediate for operand 2");
        return NULL;
    }

    // direction was pre-calculated
    i->direction_regmem_to_regimm = dir_1_to_2;

    return l;
}

asm_line *new_asm_line_instruction_for_reserving_stack_space(mempool *mp, int size) {
    asm_line *l = new_asm_line_instruction(mp, OC_SUB);

    asm_instruction *i = l->per_type.instruction;
    i->direction_regmem_to_regimm = false;
    i->operands_data_size = DATA_QWORD;
    i->regmem_operand.is_register = true;
    i->regmem_operand.per_type.reg = REG_RSP;
    i->regimm_operand.is_immediate = true;
    i->regimm_operand.per_type.immediate = (((size + 7) / 8) * 8); // round size up to 8 bytes

    return l;
}

asm_line *new_asm_line_instruction_for_register(mempool *mp, instr_code op, gp_register gp_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->regmem_operand.is_register = true;
    i->regmem_operand.per_type.reg = gp_reg;

    return l;
}

asm_line *new_asm_line_instruction_reg_reg(mempool *mp, instr_code op, gp_register target_reg, gp_register source_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->direction_regmem_to_regimm = false;
    i->regmem_operand.is_register = true;
    i->regmem_operand.per_type.reg = target_reg;
    i->regimm_operand.is_register = true;
    i->regimm_operand.per_type.reg = source_reg;

    return l;
}

asm_line *new_asm_line_instruction_reg_imm(mempool *mp, instr_code op, gp_register target_reg, long immediate) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->direction_regmem_to_regimm = false;
    i->regmem_operand.is_register = true;
    i->regmem_operand.per_type.reg = target_reg;
    i->regimm_operand.is_immediate = true;
    i->regimm_operand.per_type.immediate = immediate;

    return l;
}

asm_line *new_asm_line_instruction_mem_reg(mempool *mp, instr_code op, gp_register ptr_reg, gp_register src_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->direction_regmem_to_regimm = false;
    i->regmem_operand.is_memory_by_reg = true;
    i->regmem_operand.per_type.reg = ptr_reg;
    i->regimm_operand.is_register = true;
    i->regimm_operand.per_type.reg = src_reg;

    return l;
}

asm_line *new_asm_line_instruction_reg_mem(mempool *mp, instr_code op, gp_register target_reg, gp_register ptr_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->direction_regmem_to_regimm = true;
    i->regmem_operand.is_memory_by_reg = true;
    i->regmem_operand.per_type.reg = ptr_reg;
    i->regimm_operand.is_register = true;
    i->regimm_operand.per_type.reg = target_reg;

    return l;
}

asm_line *new_asm_line_instruction_mem_imm(mempool *mp, instr_code op, gp_register ptr_reg, data_size data_bits, long immediate) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->operands_data_size = data_bits; // e.g. "DWORD PTR"
    i->direction_regmem_to_regimm = false;
    i->regmem_operand.is_memory_by_reg = true;
    i->regmem_operand.per_type.mem.pointer_reg = ptr_reg;
    i->regimm_operand.is_immediate = true;
    i->regimm_operand.per_type.immediate = immediate;

    return l;
}

// ------------------------------------------------------------

// for working with specific values, e.g. SUB SP, <bytes>
asm_operand *new_asm_operand_imm(mempool *mp, int value) {
    asm_operand *op = mpalloc(mp, asm_operand);
    op->type = OT_IMMEDIATE;
    op->immediate = value;
    return op;
}

// for handling specific registers, e.g. BP, SP, AX
asm_operand *new_asm_operand_reg(mempool *mp, gp_register gp_reg_no) {
    asm_operand *op = mpalloc(mp, asm_operand);
    op->type = OT_REGISTER;
    op->reg = gp_reg_no;
    return op;
}

asm_operand *new_asm_operand_mem_by_sym(mempool *mp, char *symbol_name) {
    asm_operand *op = mpalloc(mp, asm_operand);
    op->type = OT_MEM_OF_SYMBOL;
    op->symbol_name = strdup(symbol_name);
    return op;
}

asm_operand *new_asm_operand_mem_by_reg(mempool *mp, gp_register gp_reg_no, int offset) {
    asm_operand *op = mpalloc(mp, asm_operand);
    op->type = OT_MEM_POINTED_BY_REG;
    op->reg = gp_reg_no;
    op->offset = offset;
    return op;
}

// ------------------------------------------------------------

data_size register_data_size(gp_register r) {
    if ((r >= REG_RAX && r <= REG_RDI) || (r >= REG_R8  && r <= REG_R15))  return DATA_QWORD;
    if ((r >= REG_EAX && r <= REG_EDI) || (r >= REG_R8D && r <= REG_R15D)) return DATA_DWORD;
    if ((r >= REG_AX  && r <= REG_DI)  || (r >= REG_R8W && r <= REG_R15W)) return DATA_WORD;
    if ((r >= REG_AL  && r <= REG_BH)  || (r >= REG_R8B && r <= REG_R15B)) return DATA_BYTE;
    return DATA_UNKNOWN;
}

int register_bits(gp_register r) {
    switch (register_data_size(r)) {
        case DATA_QWORD: return 64;
        case DATA_DWORD: return 32;
        case DATA_WORD: return 16;
        case DATA_BYTE: return 8;
    }
    return 0;
}

bool register_is_extended(gp_register r) {
    return (r >= REG_R9  && r <= REG_R15) 
        || (r >= REG_R9B && r <= REG_R15B)
        || (r >= REG_R9W && r <= REG_R15W)
        || (r >= REG_R9D && r <= REG_R15D);
}

const char *register_name(gp_register r) {
    switch (r) {
        case REG_AL: return "AL";
        case REG_CL: return "CL";
        case REG_DL: return "DL";
        case REG_BL: return "BL";
        case REG_AH: return "AH";
        case REG_CH: return "CH";
        case REG_DH: return "DH";
        case REG_BH: return "BH";

        case REG_AX: return "AX";
        case REG_CX: return "CX";
        case REG_DX: return "DX";
        case REG_BX: return "BX";
        case REG_SP: return "SP";
        case REG_BP: return "BP";
        case REG_SI: return "SI";
        case REG_DI: return "DI";

        case REG_EAX: return "EAX";
        case REG_ECX: return "ECX";
        case REG_EDX: return "EDX";
        case REG_EBX: return "EBX";
        case REG_ESP: return "ESP";
        case REG_EBP: return "EBP";
        case REG_ESI: return "ESI";
        case REG_EDI: return "EDI";

        case REG_RAX: return "RAX";
        case REG_RCX: return "RCX";
        case REG_RDX: return "RDX";
        case REG_RBX: return "RBX";
        case REG_RSP: return "RSP";
        case REG_RBP: return "RBP";
        case REG_RSI: return "RSI";
        case REG_RDI: return "RDI";

        case REG_R8B : return "R8B";
        case REG_R9B : return "R9B";
        case REG_R10B: return "R10B";
        case REG_R11B: return "R11B";
        case REG_R12B: return "R12B";
        case REG_R13B: return "R13B";
        case REG_R14B: return "R14B";
        case REG_R15B: return "R15B";

        case REG_R8W : return "R8W";
        case REG_R9W : return "R9W";
        case REG_R10W: return "R10W";
        case REG_R11W: return "R11W";
        case REG_R12W: return "R12W";
        case REG_R13W: return "R13W";
        case REG_R14W: return "R14W";
        case REG_R15W: return "R15W";

        case REG_R8D : return "R8D";
        case REG_R9D : return "R9D";
        case REG_R10D: return "R10D";
        case REG_R11D: return "R11D";
        case REG_R12D: return "R12D";
        case REG_R13D: return "R13D";
        case REG_R14D: return "R14D";
        case REG_R15D: return "R15D";

        case REG_R8 : return "R8";
        case REG_R9 : return "R9";
        case REG_R10: return "R10";
        case REG_R11: return "R11";
        case REG_R12: return "R12";
        case REG_R13: return "R13";
        case REG_R14: return "R14";
        case REG_R15: return "R15";
    }
    return "(unknown register)";
}

// -----------------------------------------------------------

data_size asm_instruction_data_size(asm_instruction *instr) {

    // reg <- reg, grab from reg, ensure same sizes
    // mem <- reg, grab size from src reg
    // reg <- mem, grab size from dest reg
    // reg <- imm, grab size from dest reg
    // mem <- imm, size must be explicit

    // notice that a register used for pointing is ignored here,
    // because the size of this pointer is irrelevant to the pointed data size.

    if (instr->regmem_operand.is_register && instr->regimm_operand.is_register) {
        // reg <--> reg
        data_size s1 = register_data_size(instr->regmem_operand.per_type.reg);
        data_size s2 = register_data_size(instr->regimm_operand.per_type.reg);
        if (s1 != s2) {
            error("reg <-> reg operations must have the same size");
            return DATA_UNKNOWN;
        }
        return s1;

    } else if ((instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) &&
                instr->regimm_operand.is_register) {
        // memory <--> register
        return register_data_size(instr->regimm_operand.per_type.reg);

    } else if (instr->regmem_operand.is_register && instr->regimm_operand.is_immediate) {
        // register <-- immediate
        return register_data_size(instr->regmem_operand.per_type.reg);

    } else if ((instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) && 
                instr->regimm_operand.is_immediate) {
        // immediate to memory, we must have explicit memory size
        return instr->operands_data_size;

    } else if (instr->regmem_operand.is_register &&
            !instr->regimm_operand.is_register && 
            !instr->regimm_operand.is_immediate) {
        // example: "PUSH RAX"
        return register_data_size(instr->regmem_operand.per_type.reg);
    } else if (!instr->regmem_operand.is_register &&
                !instr->regmem_operand.is_memory_by_reg &&
                !instr->regmem_operand.is_mem_addr_by_symbol &&
                !instr->regimm_operand.is_immediate &&
                !instr->regimm_operand.is_register) {
        // example: "RET"
        return DATA_UNKNOWN;

    } else if ((instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) && 
                !instr->regimm_operand.is_immediate && !instr->regimm_operand.is_register) {
        // e.g. "PUSH [RDX]"
        return instr->operands_data_size;
    }

    fatal("Unexpected operands configuration!");
    return DATA_UNKNOWN;
}

data_size asm_instruction_pointer_size(asm_instruction *instr) {
    if (instr->regmem_operand.is_memory_by_reg) {
        // only case pointer is relevant
        return register_data_size(instr->regmem_operand.per_type.reg);
    }

    return DATA_UNKNOWN;
}

bool asm_instruction_has_immediate(asm_instruction *instr) {
    return instr->regimm_operand.is_immediate;
}

// ------------------------------------------------------------

static void append_operand_instruction(asm_operand *op, char *buffer, int buff_size) {
    char *pos = buffer + strlen(buffer);
    int len = buff_size - strlen(buffer);

    if (op->type == OT_IMMEDIATE) {
        snprintf(pos, len, "0x%lx", op->immediate);
    } else if (op->type == OT_REGISTER) {
        snprintf(pos, len, "%c%s", run_info->options->register_prefix, register_name(op->reg));
    } else if (op->type == OT_MEM_POINTED_BY_REG) {
        snprintf(pos, len, "[%c%s%+ld]", run_info->options->register_prefix, register_name(op->reg), op->offset);
    } else if (op->type == OT_MEM_OF_SYMBOL) {
        snprintf(pos, len, "%s", op->symbol_name);
    }
}

const char *instr_code_name(instr_code code) {
    switch (code) {
        case OC_NOP: return "NOP"; 
        case OC_MOV: return "MOV"; 
        case OC_PUSH: return "PUSH"; 
        case OC_POP: return "POP"; 
        case OC_LEA: return "LEA";
        case OC_ADD: return "ADD"; 
        case OC_SUB: return "SUB";
        case OC_INC: return "INC";
        case OC_DEC: return "DEC";
        case OC_MUL: return "MUL";
        case OC_DIV: return "DIV";
        case OC_AND: return "AND";
        case OC_OR: return "OR";
        case OC_XOR: return "XOR";
        case OC_NOT: return "NOT";
        case OC_NEG: return "NEG";
        case OC_SHL: return "SHL";
        case OC_SHR: return "SHR";
        case OC_JMP: return "JMP";
        case OC_CMP: return "CMP";
        case OC_JEQ: return "JEQ";
        case OC_JNE: return "JNE";
        case OC_JAB: return "JAB";
        case OC_JAE: return "JAE";
        case OC_JBL: return "JBL";
        case OC_JBE: return "JBE";
        case OC_JGT: return "JGT";
        case OC_JGE: return "JGE";
        case OC_JLT: return "JLT";
        case OC_JLE: return "JLE";
        case OC_CALL: return "CALL";
        case OC_RET: return "RET";
        case OC_INT: return "INT";
    }
    return "(unknown)";
}

// ---------------------------------------------------------------

static void _regmem_operand_to_str(asm_instruction *instr, str *str) {
    if (instr->regmem_operand.is_register) {
        str_cats(str, register_name(instr->regmem_operand.per_type.reg));

    } else {
        // it's memory, print address size if existing
        if      (instr->operands_data_size == DATA_BYTE ) str_cats(str, "BYTE PTR ");
        else if (instr->operands_data_size == DATA_WORD ) str_cats(str, "WORD PTR ");
        else if (instr->operands_data_size == DATA_DWORD) str_cats(str, "DWORD PTR ");
        else if (instr->operands_data_size == DATA_QWORD) str_cats(str, "QWORD PTR ");

        if (instr->regmem_operand.is_mem_addr_by_symbol) {
            if (instr->regmem_operand.per_type.mem.displacement_symbol_name != NULL)
                str_cats(str, instr->regmem_operand.per_type.mem.displacement_symbol_name);
            else
                str_catf(str, "0x%lx", instr->regmem_operand.per_type.mem.displacement);

        } else if (instr->regmem_operand.is_memory_by_reg) {
            str_catf(str, "[%s", register_name(instr->regmem_operand.per_type.mem.pointer_reg));
            if (instr->regmem_operand.per_type.mem.array_item_size > 0)
                str_catf(str, "+%s*%d", 
                    register_name(instr->regmem_operand.per_type.mem.array_index_reg),
                    instr->regmem_operand.per_type.mem.array_item_size);
            if (instr->regmem_operand.per_type.mem.displacement != 0)
                str_catf(str, "%+ld", instr->regmem_operand.per_type.mem.displacement);
            str_cats(str, "]");
        }
    }
}

static void _regimm_operand_to_str(asm_instruction *instr, str *str) {
    if (instr->regimm_operand.is_register) {
        str_cats(str, register_name(instr->regimm_operand.per_type.reg));
    } else if (instr->regimm_operand.is_immediate) {
        str_catf(str, "0x%lx", (unsigned)instr->regimm_operand.per_type.immediate);
    }
}

static void _instruction_to_str(asm_instruction *instr, str *str) {
    // instr_code first
    str_catf(str, "%-4s ", instr_code_name(instr->operation));

    // if there are two operands, print direction
    if ((instr->regmem_operand.is_register || instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) &&
        (instr->regimm_operand.is_immediate || instr->regimm_operand.is_register)) {

        if (instr->direction_regmem_to_regimm) {
            _regimm_operand_to_str(instr, str);
            str_cats(str, ", ");
            _regmem_operand_to_str(instr, str);
        } else {
            _regmem_operand_to_str(instr, str);
            str_cats(str, ", ");
            _regimm_operand_to_str(instr, str);
        }

    } else {
        // any one of them
        _regmem_operand_to_str(instr, str);
        _regimm_operand_to_str(instr, str);
    }
}

str *asm_line_to_str(mempool *mp, asm_line *line) {
    str *s = new_str(mp, NULL);

    switch (line->type) {
        case ALT_EMPTY:
            break;
        case ALT_SECTION:
            str_cats(s, "section ");
            str_cat(s, line->per_type.named_definition->name);
            break;
        case ALT_EXTERN:
            str_cats(s, "extern ");
            str_cat(s, line->per_type.named_definition->name);
            break;
        case ALT_GLOBAL:
            str_cats(s, "global ");
            str_cat(s, line->per_type.named_definition->name);
            break;
        case ALT_DATA:
            str_cat(s, line->per_type.data_definition->name);
            switch (line->per_type.data_definition->unit_size) {
                case DATA_BYTE: str_cats(s, " db "); break;
                case DATA_WORD: str_cats(s, " dw "); break;
                case DATA_DWORD: str_cats(s, " dd "); break;
                case DATA_QWORD: str_cats(s, " dq "); break;
            }
            str_catf(s, "%d", line->per_type.data_definition->length_units);
            // could also dump initial value
            if (line->per_type.data_definition->initial_value != NULL) {
                str_cats(s, " = ");
                str_cat(s, bin_to_readable_bytes_str(line->per_type.data_definition->initial_value, mp));
            }            
            break;
        case ALT_INSTRUCTION:
            if (line->label != NULL) {
                str_cat(s, line->label);
                str_cats(s, ": ");
            }
            s = str_padr(s, 12, ' ');

            if (line->per_type.instruction->operation != OC_NONE)
                _instruction_to_str(line->per_type.instruction, s);
            break;
    }

    if (line->comment != NULL) {
        if (str_len(s) > 0)
            s = str_padr(s, 60, ' ');
        str_cats(s, "; ");
        str_cat(s, line->comment);
    }

    return s;
}