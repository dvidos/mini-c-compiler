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
    i->direction_op1_to_op2 = true;
    i->operands_size_bits = 64;

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
        i->operand1.is_register = true;
        i->operand1.per_type.reg = target->reg;

    } else if (target->type == OT_MEM_POINTED_BY_REG) {
        i->operand1.is_memory_by_reg = true;
        i->operand1.per_type.mem.pointer_reg = target->reg;
        i->operand1.per_type.mem.displacement = target->offset;

    } else if (target->type == OT_MEM_OF_SYMBOL) {
        i->operand1.is_mem_addr_by_symbol = true;
        i->operand1.per_type.mem.displacement_symbol_name = strdup(target->symbol_name);

    } else if (target->type == OT_IMMEDIATE) {
        // immediates go to operand2, and usually coded without the ModRM byte
        i->operand2.is_immediate = true;
        i->operand2.per_type.immediate = target->immediate;
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
        i->operand1.is_register = true;
        i->operand1.per_type.reg = operand1->reg;
    } else if (operand1->type == OT_MEM_POINTED_BY_REG) {
        i->operand1.is_memory_by_reg = true;
        i->operand1.per_type.mem.pointer_reg = REG_BP;
        i->operand1.per_type.mem.displacement = operand1->offset;
    } else if (operand1->type == OT_MEM_OF_SYMBOL) {
        i->operand1.is_mem_addr_by_symbol = true;
        i->operand1.per_type.mem.displacement_symbol_name = strdup(operand1->symbol_name);
    } else {
        error("possible bug, expected register or memory for operand 1");
        return NULL;
    }

    // operand2 should be either register or immediate
    if (operand2->type == OT_REGISTER) {
        i->operand2.is_register = true;
        i->operand2.per_type.reg = operand2->reg;
    } else if (operand2->type == OT_IMMEDIATE) {
        i->operand2.is_immediate = true;
        i->operand2.per_type.immediate = operand2->immediate;
    } else {
        error("possible bug, expected register or immediate for operand 2");
        return NULL;
    }

    // direction was pre-calculated
    i->direction_op1_to_op2 = dir_1_to_2;

    return l;
}

asm_line *new_asm_line_instruction_for_reserving_stack_space(mempool *mp, int size) {
    asm_line *l = new_asm_line_instruction(mp, OC_SUB);

    asm_instruction *i = l->per_type.instruction;
    i->direction_op1_to_op2 = false;
    i->operands_size_bits = 32;
    i->operand1.is_register = true;
    i->operand1.per_type.reg = REG_SP;
    i->operand2.is_immediate = true;
    i->operand2.per_type.immediate = (((size + 3) / 4) * 4); // round size up to 4 bytes

    return l;
}

asm_line *new_asm_line_instruction_for_register(mempool *mp, instr_code op, gp_register gp_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->operand1.is_register = true;
    i->operand1.per_type.reg = gp_reg;

    return l;
}

asm_line *new_asm_line_instruction_for_registers(mempool *mp, instr_code op, gp_register target_reg, gp_register source_reg) {
    asm_line *l = new_asm_line_instruction(mp, op);

    asm_instruction *i = l->per_type.instruction;
    i->direction_op1_to_op2 = true;
    i->operand1.is_register = true;
    i->operand1.per_type.reg = source_reg;
    i->operand2.is_register = true;
    i->operand2.per_type.reg = target_reg;

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


static void append_operand_instruction(asm_operand *op, char *buffer, int buff_size) {
    char *pos = buffer + strlen(buffer);
    int len = buff_size - strlen(buffer);

    if (op->type == OT_IMMEDIATE) {
        snprintf(pos, len, "0x%lx", op->immediate);
    } else if (op->type == OT_REGISTER) {
        snprintf(pos, len, "%c%s", run_info->options->register_prefix, gp_reg_name(op->reg));
    } else if (op->type == OT_MEM_POINTED_BY_REG) {
        snprintf(pos, len, "[%c%s%+ld]", run_info->options->register_prefix, gp_reg_name(op->reg), op->offset);
    } else if (op->type == OT_MEM_OF_SYMBOL) {
        snprintf(pos, len, "%s", op->symbol_name);
    }
}

char *instr_code_name(instr_code code) {
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

char *gp_reg_name(gp_register r) {
    switch (r) {
        case REG_AX: return "AX";
        case REG_CX: return "CX";
        case REG_DX: return "DX";
        case REG_BX: return "BX";
        case REG_SP: return "SP";
        case REG_BP: return "BP";
        case REG_SI: return "SI";
        case REG_DI: return "DI";
    }
    return "(unknown)";
}


// ---------------------------------------------------------------




static void _operand1_to_str(asm_instruction *instr, str *str) {
    if (instr->operand1.is_register) {
        str_cats(str, gp_reg_name(instr->operand1.per_type.reg));

    } else if (instr->operand1.is_mem_addr_by_symbol) {
        if (instr->operand1.per_type.mem.displacement_symbol_name != NULL)
            str_cats(str, instr->operand1.per_type.mem.displacement_symbol_name);
        else {
            str_catf(str, "0x%lx", instr->operand1.per_type.mem.displacement);

        }

    } else if (instr->operand1.is_memory_by_reg) {
        str_catf(str, "[%s", gp_reg_name(instr->operand1.per_type.mem.pointer_reg));
        if (instr->operand1.per_type.mem.array_item_size > 0)
            str_catf(str, "+%s*%d", 
                gp_reg_name(instr->operand1.per_type.mem.array_index_reg),
                instr->operand1.per_type.mem.array_item_size);
        if (instr->operand1.per_type.mem.displacement != 0)
            str_catf(str, "%+ld", instr->operand1.per_type.mem.displacement);
        str_cats(str, "]");
    }
}

static void _operand2_to_str(asm_instruction *instr, str *str) {
    if (instr->operand2.is_register) {
        str_cats(str, gp_reg_name(instr->operand2.per_type.reg));
    } else if (instr->operand2.is_immediate) {
        str_catf(str, "0x%lx", (unsigned)instr->operand2.per_type.immediate);
    }
}

static void _instruction_to_str(asm_instruction *instr, str *str) {
    // instr_code first
    str_catf(str, "%-4s ", instr_code_name(instr->operation));

    // if there are two operands, print direction
    if ((instr->operand1.is_register || instr->operand1.is_memory_by_reg || instr->operand1.is_mem_addr_by_symbol) &&
        (instr->operand2.is_immediate || instr->operand2.is_register)) {

        if (instr->direction_op1_to_op2) {
            _operand2_to_str(instr, str);
            str_cats(str, ", ");
            _operand1_to_str(instr, str);
        } else {
            _operand1_to_str(instr, str);
            str_cats(str, ", ");
            _operand2_to_str(instr, str);
        }

    } else {
        // any one of them
        _operand1_to_str(instr, str);
        _operand2_to_str(instr, str);
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