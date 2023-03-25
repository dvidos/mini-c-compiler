#include <string.h>
#include <stdlib.h>
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
    char tmp[64];

    fprintf(stream, "%-10s %d-bit  ", opcode_name(oper->operation), oper->operands_size_bits);
    if (oper->operand1.is_register) {
        fprintf(stream, "%s", gp_reg_name(oper->operand1.per_type.reg));
    } else if (oper->operand1.is_memory_by_displacement) {
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
}

asm_operation *new_asm_operation(enum opcode op) {
    asm_operation *p = malloc(sizeof(asm_operation));
    memset(p, 0, sizeof(asm_operation));

    p->operation = op;
    p->operands_size_bits = 32;
    p->direction_op1_to_op2 = true;

    return p;
}