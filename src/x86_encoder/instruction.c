#include <string.h>
#include <stdio.h>
#include "instruction.h"
#include "../options.h"



static char *get_opcode_str(enum opcode code);
static char *get_reg_str(enum reg r);
static void append_operand_instruction(struct operand *op, char *buffer, int buff_size);


void instruction_to_string(struct instruction *inst, char *buff, int buff_size) {
    strncpy(buff, get_opcode_str(inst->opcode), buff_size);
    if (inst->op1.type != OT_NONE) {
        strncat(buff, " ", buff_size);
        append_operand_instruction(&inst->op1, buff, buff_size);
        if (inst->op2.type != OT_NONE) {
            strncat(buff, ", ", buff_size);
            append_operand_instruction(&inst->op2, buff, buff_size);
        }
    }
}

static void append_operand_instruction(struct operand *op, char *buffer, int buff_size) {
    char *pos = buffer + strlen(buffer);
    int len = buff_size - strlen(buffer);

    if (op->type == OT_NONE) {
        ; // nothing.
    } else if (op->type == OT_IMMEDIATE) {
        snprintf(pos, len, "0x%lx", op->value);
    } else if (op->type == OT_REGISTER) {
        snprintf(pos, len, "%c%s", options.register_prefix, get_reg_str(op->value));
    } else if (op->type == OT_MEM_DWORD_POINTED_BY_REG) {
        if (op->offset < 0) {
            snprintf(pos, len, "[%c%s%ld]", options.register_prefix, get_reg_str(op->value), op->offset);
        } else if (op->offset > 0) {
            snprintf(pos, len, "[%c%s+%ld]", options.register_prefix, get_reg_str(op->value), op->offset);
        } else {
            snprintf(pos, len, "[%c%s]", options.register_prefix, get_reg_str(op->value));
        }
    } else if (op->type == OT_SYMBOL_MEM_ADDRESS) {
        snprintf(pos, len, "%s", op->symbol_name);
    }
}

static char *get_opcode_str(enum opcode code) {
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
        case OC_IMUL: return "IMUL";
        case OC_IDIV: return "IDIV";
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

static char *get_reg_str(enum reg r) {
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
