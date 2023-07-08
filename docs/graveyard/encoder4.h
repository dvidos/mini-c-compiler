#pragma once
#include <stdio.h>
#include <stdbool.h>
#include "asm_line.h"
#include "encoded_instruction.h"
#include "encoding_info.h"


/*
    essentially, since the instruction encoding is so complex
    we need to find a simpler way to approach it.
    all the documentation is useful for disassembling, but does
    not help how to go about choosing how to assemble something.
    
    Displacements could/should be used for structure members.
    SIBs could/should be used for arrays of 1/2/4/8 bytes elements

    But all the above complexity can go away, by code that 
    calculates the exact memory address and puts it in a register.

    For immediate constants, things are even more complex
    Operands have the most significant bit 1,
    Mod+RM have different meanings, direction bit is different, etc.

    Maybe we should start amazingly simple.
*/

bool encode_asm_instruction(asm_instruction *inst, struct encoding_info *info, struct encoded_instruction *result);

