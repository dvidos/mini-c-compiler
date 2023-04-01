#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "asm_instruction.h"
#include "../../options.h"
#include "../../err_handler.h"


static void append_operand_instruction(struct asm_operand *op, char *buffer, int buff_size);


void instruction_old_to_string(struct asm_instruction_old *inst, char *buff, int buff_size) {
    strncpy(buff, opcode_name(inst->opcode), buff_size);
    if (inst->op1 != NULL) {
        strncat(buff, " ", buff_size);
        append_operand_instruction(inst->op1, buff, buff_size);
        if (inst->op2 != NULL) {
            strncat(buff, ", ", buff_size);
            append_operand_instruction(inst->op2, buff, buff_size);
        }
    }
}

static void append_operand_instruction(struct asm_operand *op, char *buffer, int buff_size) {
    char *pos = buffer + strlen(buffer);
    int len = buff_size - strlen(buffer);

    if (op->type == OT_IMMEDIATE) {
        snprintf(pos, len, "0x%lx", op->immediate);
    } else if (op->type == OT_REGISTER) {
        snprintf(pos, len, "%c%s", options.register_prefix, gp_reg_name(op->reg));
    } else if (op->type == OT_MEM_POINTED_BY_REG) {
        snprintf(pos, len, "[%c%s%+ld]", options.register_prefix, gp_reg_name(op->reg), op->offset);
    } else if (op->type == OT_MEM_OF_SYMBOL) {
        snprintf(pos, len, "%s", op->symbol_name);
    }
}

char *opcode_name(enum opcode code) {
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
        case OC_MUL: return "IMUL";
        case OC_DIV: return "IDIV";
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

char *gp_reg_name(enum gp_reg r) {
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

asm_instruction *new_asm_instruction(enum opcode op) {
    asm_instruction *p = malloc(sizeof(asm_instruction));
    memset(p, 0, sizeof(asm_instruction));

    p->operation = op;
    p->operands_size_bits = 32;
    p->direction_op1_to_op2 = true;

    return p;
}

asm_instruction *new_asm_instruction_with_operand(enum opcode op, struct asm_operand *target) {

    asm_instruction *o = new_asm_instruction(op);
    
    // operand1 should be either register or memory
    if (target->type == OT_REGISTER) {
        o->operand1.is_register = true;
        o->operand1.per_type.reg = target->reg;

    } else if (target->type == OT_MEM_POINTED_BY_REG) {
        o->operand1.is_memory_by_reg = true;
        o->operand1.per_type.mem.pointer_reg = REG_BP;
        o->operand1.per_type.mem.displacement = target->offset;

    } else if (target->type == OT_MEM_OF_SYMBOL) {
        o->operand1.is_memory_by_displacement = true;
        o->operand1.per_type.mem.displacement_symbol_name = strdup(target->symbol_name);

    } else if (target->type == OT_IMMEDIATE) {
        // immediates go to operand2, and usually coded without the ModRM byte
        o->operand2.is_immediate = true;
        o->operand2.per_type.immediate = target->immediate;
    }

    // we should solve the operands size bits, some day...
    // we should also save pointers to the operands for later freeing
    return o;
}

asm_instruction *new_asm_instruction_with_operands(enum opcode op, struct asm_operand *target, struct asm_operand *source) {

    // operations memory to memory, or any to immediate, are not supported
    if ((source->type == OT_MEM_OF_SYMBOL || source->type == OT_MEM_POINTED_BY_REG) &&
        (target->type == OT_MEM_OF_SYMBOL || target->type == OT_MEM_POINTED_BY_REG)) {
        error(NULL, 0, "mem-to-mem operations cannot be codified, use of register needed");
        return NULL;
    }
    if (target->type == OT_IMMEDIATE) {
        error(NULL, 0, "operations towards immediates cannot be codified, use of register needed");
        return NULL;
    }
    
    // target_v   source_v   operand1   operand2    direction
    // --------------------------------------------------------
    // reg        reg        reg (tv)   reg (sv)    op2 to op1
    // reg        imm        reg (tv)   imm (sv)    op2 to op1
    // reg        mem        mem (sv)   reg (tv)    op1 to op2 <-- only case 1 to 2
    // mem        reg        mem (tv)   reg (sv)    op2 to op1
    // mem        imm        mem (tv)   imm (sv)    op2 to op1

    struct asm_operand *operand1;
    struct asm_operand *operand2;
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
    
    asm_instruction *o = new_asm_instruction(op);
    
    // operand1 should be either register or memory
    if (operand1->type == OT_REGISTER) {
        o->operand1.is_register = true;
        o->operand1.per_type.reg = operand1->reg;
    } else if (operand1->type == OT_MEM_POINTED_BY_REG) {
        o->operand1.is_memory_by_reg = true;
        o->operand1.per_type.mem.pointer_reg = REG_BP;
        o->operand1.per_type.mem.displacement = operand1->offset;
    } else if (operand1->type == OT_MEM_OF_SYMBOL) {
        o->operand1.is_memory_by_displacement = true;
        o->operand1.per_type.mem.displacement_symbol_name = strdup(operand1->symbol_name);
    } else {
        error(NULL, 0, "possible bug, expected register or memory for operand 1");
        return NULL;
    }

    // operand2 should be either register or immediate
    if (operand2->type == OT_REGISTER) {
        o->operand2.is_register = true;
        o->operand2.per_type.reg = operand2->reg;
    } else if (operand2->type == OT_IMMEDIATE) {
        o->operand2.is_immediate = true;
        o->operand2.per_type.immediate = operand2->immediate;
    } else {
        error(NULL, 0, "possible bug, expected register or immediate for operand 2");
        return NULL;
    }

    // direction was pre-calculated
    o->direction_op1_to_op2 = dir_1_to_2;
    
    // we should solve the operands size bits, some day...
    // we should also save pointers to the operands for later freeing
    return o;
}


asm_instruction *new_asm_instruction_for_reserving_stack_space(int size) {
    asm_instruction *p = malloc(sizeof(asm_instruction));
    memset(p, 0, sizeof(asm_instruction));

    p->operation = OC_SUB;
    p->direction_op1_to_op2 = true;
    p->operands_size_bits = 32;
    p->operand1.is_register = true;
    p->operand1.per_type.reg = REG_SP;
    p->operand2.is_immediate = true;
    p->operand2.per_type.immediate = size;

    return p;
}

asm_instruction *new_asm_instruction_for_register(enum opcode op, enum gp_reg gp_reg) {
    asm_instruction *o = new_asm_instruction(op);

    o->operand1.is_register = true;
    o->operand1.per_type.reg = gp_reg;

    return o;
}

asm_instruction *new_asm_instruction_for_registers(enum opcode op, enum gp_reg target_reg, enum gp_reg source_reg) {
    asm_instruction *o = new_asm_instruction(op);

    o->direction_op1_to_op2 = true;
    o->operand1.is_register = true;
    o->operand1.per_type.reg = source_reg;
    o->operand2.is_register = true;
    o->operand2.per_type.reg = target_reg;

    return o;
}

void asm_instruction_to_str(asm_instruction *instr, str *str) {


    // Mnemonic  Size   Operand1                  Dir    Operand2
    // XXXXXXX   dword  [EAX+ECX*8+0x12345678]    <--    EDX / 0x12345678
    str->v->clear(str);

    if (instr->label)
        str->v->addf(str, "%s:\n", instr->label);
    str->v->addf(str, "%-10s %d-bits  ", opcode_name(instr->operation), instr->operands_size_bits);

    // first operand: register or memory
    if (instr->operand1.is_register) {
        str->v->addz(str, gp_reg_name(instr->operand1.per_type.reg));
    } else if (instr->operand1.is_memory_by_displacement) {
        if (instr->operand1.per_type.mem.displacement_symbol_name != NULL)
            str->v->addz(str, instr->operand1.per_type.mem.displacement_symbol_name);
        else
            str->v->addf(str, "0x%lx", instr->operand1.per_type.mem.displacement);
    } else if (instr->operand1.is_memory_by_reg) {
        str->v->addf(str, "[%s", gp_reg_name(instr->operand1.per_type.mem.pointer_reg));
        if (instr->operand1.per_type.mem.array_item_size > 0)
            str->v->addf(str, "+%s*%d", gp_reg_name(instr->operand1.per_type.mem.array_index_reg),
                instr->operand1.per_type.mem.array_item_size);
        if (instr->operand1.per_type.mem.displacement > 0)
            str->v->addf(str, "%+ld", instr->operand1.per_type.mem.displacement);
        str->v->addz(str, "]");
    }

    // direction
    str->v->addf(str, " %s ", instr->direction_op1_to_op2 ? "-->" : "<--");

    // operand 2: register or immediate
    if (instr->operand2.is_register) {
        str->v->addf(str, "%s", gp_reg_name(instr->operand1.per_type.reg));
    } else if (instr->operand2.is_immediate) {
        str->v->addf(str, "0x%lx", instr->operand2.per_type.immediate);
    }

    // possible comment
    if (instr->comment)
        str->v->addf(str, " ; %s", instr->comment);
}
