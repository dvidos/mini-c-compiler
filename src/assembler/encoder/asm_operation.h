#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "asm_instruction.h"
#include "encoded_instruction.h"
#include "encoding_info.h"


typedef struct asm_operation {
    enum opcode operation; // ADD, SUB, etc. no sign cognizance
    int operands_size_bits; // 8,16,32,64 (width bit for 8bits, 0x66 prefix for 16bits)
    bool direction_op1_to_op2; // if false, the opposite

    // register or memory
    // goes to the Mod+R/M part of the ModRegRM byte
    struct operand1 { 
        bool is_register;
        bool is_memory_by_reg;
        bool is_memory_by_displacement; // i.e. a symbol
        union {
            enum gp_reg reg;
            struct {
                enum gp_reg pointer_reg;
                long displacement;   // 0 means no displacement
                enum gp_reg array_index_reg;
                int array_item_size; // must be 1,2,4 or 8 for SID to be used
            } mem;
        } per_type;
    } operand1; 

    // register or immediate.
    // goes to the Reg part of the Mod+Reg+RM byte
    struct operand2 {  
        bool is_immediate;
        bool is_register;
        union {
            long immediate;
            enum gp_reg reg;
        } per_type;
    } operand2; 
} asm_operation;

void print_asm_operation(asm_operation *oper, FILE *stream);
asm_operation *new_asm_operation(enum opcode op);
