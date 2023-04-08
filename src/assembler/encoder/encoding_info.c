#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "encoding_info.h"


// table assembled by hand, by reading Intel Manual tables and documentation and testing
// table string format:
// dots or spaces are false, "y" or "1" are true
// spaces means N/A, digits are hex (i.e. 2 digits for 1 byte)

// see encoding rows
#define INSTR_EXPANSION_BYTE_POS   0
#define MAIN_OPCODE_POS            3
#define WIDTH_BIT_POS              6
#define DIRECTION_BIT_POS          7
#define REGISTER_PART_POS          8
#define OPCODE_EXTENSION_POS       11 // position right after the possible slash
#define SUPPORTS_IMMEDIATE_POS     13
#define NEEDS_MODREGRM_POS         14
#define HAS_DISPLACEMENT_POS       15
#define SIGN_EXPANDED_POS          16

#define HAS_BYTE(str, pos)         (str[pos] != ' ' && str[pos] != '.' && str[pos] != '_')
#define HEX_DIGIT(chr)             (((chr) <= '9') ? (chr) - '0' : toupper(chr) - 'A' + 10)
#define GET_BYTE(str, pos)         ((HEX_DIGIT(str[pos]) << 4) | HEX_DIGIT(str[pos + 1]))
#define IS_TRUE(str, pos)          (str[pos] == '1' || str[pos] == 'y' || str[pos] == 'Y')

struct encoding_info_table_row {
    enum opcode op;
    char *table_str;
} encoding_rows[] = {
            //  +----------------- instruction expansion byte (spaces = n/a, usually not needed)
            //  |  +-------------- main opcode
            //  |  |
            //  |  |  +----------- has width bit (bit0)
            //  |  |  |+---------- has direction bit (bit1)
            //  |  |  ||+--------- register part of opcode 3 least significant bits
            //  |  |  |||  +------ opcode extension (e.g. "/6", space = n/a)
            //  |  |  |||  |
            //  |  |  |||  | +---- supports immediate value (many bytes, unless sign expanded bit is supported & on)
            //  |  |  |||  | |+--- needs ModRegRm byte
            //  |  |  |||  | ||+-- has displacement without ModRmReg
            //  |  |  |||  | |||+-- has sign expanded immediate bit (bit1)
            //  |  |  |||  | ||||
            //  XX XX yyy /n yyyy   
            //  012345678901234567
    { OC_NONE, "   .. ... .. .... " },
    { OC_NOP,  "   90          .  " },
    { OC_PUSH, "   FF ... /6 .y.. " }, // essentially 0xFF and a ModRM, short hand would be '01010rrr'
    { OC_PUSH, "   68 ... /. y..y " }, // for pushing immediates
    { OC_POP,  "   8F ... /0 .y.. " }, // essentially 0x8F and a ModRM, short hand would be '01011rrr'
    { OC_MOV,  "   88 yy. /. .y.. " }, // move between reg, mem
    { OC_MOV,  "   C6 y.. /0 yy.. " }, // for immediates (target in modregrm: mem or reg)
    { OC_RET,  "   C3 ... /. .... " }, // return inside segment
    { OC_CALL, "   E8 ... /. ...y " }, // direct call, full displacement (4bytes)
    { OC_CALL, "   FF ... /2 .y.. " }, // indirect call, through register
    { OC_CMP,  "   34 yy. /. .y.. " }, // move between reg, mem
    { OC_CMP,  "   80 yy. /7 yy.. " }, // for immediates (target in modregrm: mem or reg)
    { OC_JMP,  "   E9 ... .. ...y " }, // direct, through symbol (resolved by linker)
    { OC_JMP,  "   FF ... /4 .y.. " }, // indirect, through register

    // the following with full displacement, e.g. 4 bytes
    { OC_JEQ,  "0F 84 ... /. ..y. " },
    { OC_JNE,  "0F 85 ... /. ..y. " },
    // the following for unsigned integers
    { OC_JAB,  "0F 87 ... /. ..y. " }, // jump if above
    { OC_JAE,  "0F 83 ... /. ..y. " }, // jump if above or equal
    { OC_JBL,  "0F 82 ... /. ..y. " }, // jump if below
    { OC_JBE,  "0F 86 ... /. ..y. " }, // jump if below or equal
    // the following for signed integers
    { OC_JGT,  "0F 8F ... /. ..y. " }, // jump if greater
    { OC_JGE,  "0F 8D ... /. ..y. " }, // jump if greater or equal
    { OC_JLT,  "0F 8C ... /. ..y. " }, // jump if less
    { OC_JLE,  "0F 8E ... /. ..y. " }, // jump if less or equal

    { OC_ADD,  "   00 yy. /. .y. " }, // add between reg, mem
    { OC_ADD,  "   80 y.. /0 yyy " }, // add immediate
    { OC_SUB,  "   28 yy. /. .y. " }, // sub between reg, mem
    { OC_SUB,  "   80 y.. /5 yyy " }, // sub immediate
    { OC_MUL,  "   F6 y.. /4 .y. " },  // multiply AX with register or memory location
    { OC_DIV,  "   F6 y.. /6 .y. " },  // divide AX with register or memory location, remainder in DX
};
/*  1001  9
    1010  A
    1011  B
    1100  C
    1101  D
    1110  E
    1111  F */

bool load_encoding_info(asm_instruction *inst, struct encoding_info *info) {
    bool needs_immediate = inst->operand2.is_immediate;
    bool needs_displacement = inst->operand1.is_memory_by_displacement;
    
    for (int i = 0; i < sizeof(encoding_rows) / sizeof(encoding_rows[0]); i++) {
        struct encoding_info_table_row *entry = &encoding_rows[i];
        if (entry->op != inst->operation)
            continue;

        // parse the info
        info->has_instruction_expansion_byte  = HAS_BYTE(entry->table_str, INSTR_EXPANSION_BYTE_POS);
        info->instruction_expansion_byte      = GET_BYTE(entry->table_str, INSTR_EXPANSION_BYTE_POS);
        info->base_opcode_byte                = GET_BYTE(entry->table_str, MAIN_OPCODE_POS);
        info->has_width_bit                   = IS_TRUE(entry->table_str, WIDTH_BIT_POS);
        info->has_direction_bit               = IS_TRUE(entry->table_str, DIRECTION_BIT_POS);
        info->has_opcode_extension            = HAS_BYTE(entry->table_str, OPCODE_EXTENSION_POS);
        info->opcode_extension_value          = HEX_DIGIT(entry->table_str[OPCODE_EXTENSION_POS]);
        info->is_reg_part_of_opcode           = IS_TRUE(entry->table_str, REGISTER_PART_POS);
        info->supports_immediate_value        = IS_TRUE(entry->table_str, SUPPORTS_IMMEDIATE_POS);
        info->needs_modregrm                  = IS_TRUE(entry->table_str, NEEDS_MODREGRM_POS);
        info->displacement_without_modrm      = IS_TRUE(entry->table_str, HAS_DISPLACEMENT_POS);
        info->has_sign_expanded_immediate_bit = IS_TRUE(entry->table_str, SIGN_EXPANDED_POS);

        if (needs_immediate && !info->supports_immediate_value)
            continue;
        if (needs_displacement && !(info->needs_modregrm || info->displacement_without_modrm))
            continue;
        
        return true;
    }

    return false;
}

