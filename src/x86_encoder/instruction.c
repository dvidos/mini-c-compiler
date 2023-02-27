#include <string.h>
#include <stdio.h>
#include "instruction.h"
#include "../options.h"



static char *get_opcode_str(enum opcode code);
static char *get_reg_str(enum reg r);
static void append_operand_instruction(struct operand *op, char *buffer);


void instruction_to_string(struct instruction *inst, char *buff) {
    strcpy(buff, get_opcode_str(inst->opcode));
    if (inst->op1.type != OT_NONE) {
        strcat(buff, " ");
        append_operand_instruction(&inst->op1, buff);
        if (inst->op2.type != OT_NONE) {
            strcat(buff, ", ");
            append_operand_instruction(&inst->op2, buff);
        }
    }
}

static void append_operand_instruction(struct operand *op, char *buffer) {
    if (op->type == OT_NONE) {
        ; // nothing.
    } else if (op->type == OT_IMMEDIATE) {
        sprintf(buffer + strlen(buffer), "0x%lx", op->value);
    } else if (op->type == OT_REGISTER) {
        sprintf(buffer + strlen(buffer), "%c%s", options.register_prefix, get_reg_str(op->value));
    } else if (op->type == OT_MEM_DWORD_POINTED_BY_REG) {
        if (op->offset < 0) {
            sprintf(buffer + strlen(buffer), "[%c%s%ld]", options.register_prefix, get_reg_str(op->value), op->offset);
        } else if (op->offset > 0) {
            sprintf(buffer + strlen(buffer), "[%c%s+%ld]", options.register_prefix, get_reg_str(op->value), op->offset);
        } else {
            sprintf(buffer + strlen(buffer), "[%c%s]", options.register_prefix, get_reg_str(op->value));
        }
    } else if (op->type == OT_SYMBOL_MEM_ADDRESS) {
        sprintf(buffer + strlen(buffer), "%s", op->symbol_name);
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
