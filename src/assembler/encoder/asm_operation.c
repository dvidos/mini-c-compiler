#include <string.h>
#include <stdlib.h>
#include "../../err_handler.h"
#include "encoder4.h"
#include "asm_instruction.h"
#include "asm_operation.h"
#include "encoding_info.h"
#include "encoded_instruction.h"



void print_asm_operation(asm_operation *oper, FILE *stream) {
/*
    Mnemonic  Size   Operand1                  Dir    Operand2
    XXXXXXX   dword  [EAX+ECX*8+0x12345678]    <--    EDX / 0x12345678
*/

    if (oper->label)
        fprintf(stream, "%s:\n", oper->label);
    
    fprintf(stream, "%-10s %d-bit  ", opcode_name(oper->operation), oper->operands_size_bits);

    if (oper->operand1.is_register) {
        fprintf(stream, "%s", gp_reg_name(oper->operand1.per_type.reg));
    } else if (oper->operand1.is_memory_by_displacement) {
        if (oper->operand1.per_type.mem.displacement_symbol_name != NULL)
            fprintf(stream, "%s", oper->operand1.per_type.mem.displacement_symbol_name);
        else
            fprintf(stream, "0x%lx", oper->operand1.per_type.mem.displacement);
    } else if (oper->operand1.is_memory_by_reg) {
        fprintf(stream, "[%s", gp_reg_name(oper->operand1.per_type.mem.pointer_reg));
        if (oper->operand1.per_type.mem.array_item_size > 0)
            fprintf(stream, "+%s*%d", gp_reg_name(oper->operand1.per_type.mem.array_index_reg),
                oper->operand1.per_type.mem.array_item_size);
        if (oper->operand1.per_type.mem.displacement > 0)
            fprintf(stream, "%+ld", oper->operand1.per_type.mem.displacement);
        fprintf(stream, "]");
    }

    // direction
    fprintf(stream, " %s ", oper->direction_op1_to_op2 ? "-->" : "<--");

    // operand 2
    if (oper->operand2.is_register) {
        fprintf(stream, "%s", gp_reg_name(oper->operand1.per_type.reg));
    } else if (oper->operand2.is_immediate) {
        fprintf(stream, "0x%lx", oper->operand2.per_type.immediate);
    }

    // possible comment
    if (oper->comment)
        fprintf(stream, " ; %s", oper->comment);
}

asm_operation *new_asm_operation(enum opcode op) {
    asm_operation *p = malloc(sizeof(asm_operation));
    memset(p, 0, sizeof(asm_operation));

    p->operation = op;
    p->operands_size_bits = 32;
    p->direction_op1_to_op2 = true;

    return p;
}

asm_operation *new_asm_operation_with_operand(enum opcode op, struct asm_operand *target) {

    asm_operation *o = new_asm_operation(op);
    
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

asm_operation *new_asm_operation_with_operands(enum opcode op, struct asm_operand *target, struct asm_operand *source) {

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
    
    asm_operation *o = new_asm_operation(op);
    
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


asm_operation *new_asm_operation_for_reserving_stack_space(int size) {
    asm_operation *p = malloc(sizeof(asm_operation));
    memset(p, 0, sizeof(asm_operation));

    p->operation = OC_SUB;
    p->direction_op1_to_op2 = true;
    p->operands_size_bits = 32;
    p->operand1.is_register = true;
    p->operand1.per_type.reg = REG_SP;
    p->operand2.is_immediate = true;
    p->operand2.per_type.immediate = size;

    return p;
}

asm_operation *new_asm_operation_for_register(enum opcode op, enum gp_reg gp_reg) {
    asm_operation *o = new_asm_operation(op);

    o->operand1.is_register = true;
    o->operand1.per_type.reg = gp_reg;

    return o;
}

asm_operation *new_asm_operation_for_registers(enum opcode op, enum gp_reg target_reg, enum gp_reg source_reg) {
    asm_operation *o = new_asm_operation(op);

    o->direction_op1_to_op2 = true;
    o->operand1.is_register = true;
    o->operand1.per_type.reg = source_reg;
    o->operand2.is_register = true;
    o->operand2.per_type.reg = target_reg;

    return o;
}