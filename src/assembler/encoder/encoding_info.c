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
#define SIGN_EXPANDED_POS          8
#define REGISTER_PART_POS          9
#define OPCODE_EXTENSION_POS       12 // position right after the possible slash
#define NEEDS_MODREGRM_POS         14
#define HAS_DISPLACEMENT_POS       15
#define IMMEDIATE_SUPPORT          17


#define HAS_BYTE(str, pos)         (str[pos] != ' ' && str[pos] != '.' && str[pos] != '_')
#define HEX_DIGIT(chr)             (((chr) <= '9') ? (chr) - '0' : toupper(chr) - 'A' + 10)
#define GET_BYTE(str, pos)         ((HEX_DIGIT(str[pos]) << 4) | HEX_DIGIT(str[pos + 1]))
#define IS_TRUE(str, pos)          (str[pos] == '1' || str[pos] == 'y' || str[pos] == 'Y')

struct encoding_info_table_row {
    instr_code op;
    char *table_str;
} encoding_rows[] = {
            //  +------------------ instruction expansion byte (spaces = n/a, usually not needed)
            //  |  +--------------- main opcode
            //  |  |
            //  |  |  +------------ has width bit (bit0)
            //  |  |  |+----------- has direction bit (bit1)
            //  |  |  ||+---------- has sign expanded immediate bit (bit1)
            //  |  |  |||+--------- register is part of opcode 3 least significant bits
            //  |  |  ||||  +------ opcode extension (e.g. "/6", space = n/a)
            //  |  |  ||||  |
            //  |  |  ||||  | +---- needs ModRegRm byte
            //  |  |  ||||  | |+--- has displacement without ModRmReg
            //  |  |  ||||  | ||  +-- immediate bytes: 08, 32, or sb (= depends on sign expansion bit)
            //  |  |  ||||  | ||  |
            //  XX XX yyyy /n yy nn
            //  012345678901234567890
    { OC_NONE, "   .. .... .. .. __ " },
    { OC_NOP,  "   90          . __ " },
    { OC_PUSH, "   FF .... /6 y. __ " }, // essentially 0xFF and a ModRM, short hand would be '01010rrr'
    { OC_PUSH, "   68 ..y. /. .. sb " }, // for pushing immediates
    { OC_POP,  "   8F .... /0 y. __ " }, // essentially 0x8F and a ModRM, short hand would be '01011rrr'
    { OC_MOV,  "   88 yy.. /. y. __ " }, // move between reg, mem
    { OC_MOV,  "   C6 y... /0 y. 32 " }, // for immediates (target in modregrm: mem or reg)
    { OC_RET,  "   C3 .... /. .. __ " }, // return inside segment
    { OC_CALL, "   E8 .... /. .y __ " }, // direct call, full displacement (4bytes)
    { OC_CALL, "   FF .... /2 y. __ " }, // indirect call, through register
    { OC_CMP,  "   34 yy.. /. y. __ " }, // move between reg, mem
    { OC_CMP,  "   80 yy.. /7 y. sb " }, // for immediates (target in modregrm: mem or reg)
    { OC_JMP,  "   E9 ..y. .. .. __ " }, // direct, through symbol (resolved by linker)
    { OC_JMP,  "   FF .... /4 y. __ " }, // indirect, through register
    { OC_JEQ,  "0F 84 .... /. .y __ " }, // the following with full displacement, e.g. 4 bytes
    { OC_JNE,  "0F 85 .... /. .y __ " },
    { OC_JAB,  "0F 87 .... /. .y __ " }, // (for unsigned ints) jump if above
    { OC_JAE,  "0F 83 .... /. .y __ " }, // (for unsigned ints) jump if above or equal
    { OC_JBL,  "0F 82 .... /. .y __ " }, // (for unsigned ints) jump if below
    { OC_JBE,  "0F 86 .... /. .y __ " }, // (for unsigned ints) jump if below or equal
    { OC_JGT,  "0F 8F .... /. .y __ " }, // (for signed ints) jump if greater
    { OC_JGE,  "0F 8D .... /. .y __ " }, // (for signed ints) jump if greater or equal
    { OC_JLT,  "0F 8C .... /. .y __ " }, // (for signed ints) jump if less
    { OC_JLE,  "0F 8E .... /. .y __ " }, // (for signed ints) jump if less or equal
    { OC_ADD,  "   00 yy.. /. y. __ " }, // add between reg, mem
    { OC_ADD,  "   80 y... /0 yy sb " }, // add immediate (32/8 bits depending on sign expansion bit)
    { OC_SUB,  "   28 yy.. /. y. __ " }, // sub between reg, mem
    { OC_SUB,  "   80 y... /5 yy sb " }, // sub immediate
    { OC_MUL,  "   F6 y... /4 y. __ " }, // multiply AX with register or memory location
    { OC_DIV,  "   F6 y... /6 y. __ " }, // divide AX with register or memory location, remainder in DX
    { OC_INT,  "   CD .... /. .. 08 " }, // a one byte immediate
    { OC_INC,  "   FF y... /0 y. __ " }, // inc's memory or register
    { OC_DEC,  "   FF y... /1 y. __ " }, // inc's memory or register
    { OC_NOT,  "   F6 y... /2 y. __ " }, // NOT's memory or register (one's complement)
    { OC_NEG,  "   F6 y... /3 y. __ " }, // NEG's memory or register (two's complement, make negative)
    { OC_AND,  "   20 yy.. /. y. __ " }, // reg from/to mem/reg
    { OC_OR,   "   08 yy.. /. y. __ " }, // reg from/to mem/reg
    { OC_XOR,  "   30 yy.. /. y. __ " }, // reg from/to mem/reg
    { OC_AND,  "   80 y.y. /4 y. sb " }, // immediate to reg/mem
    { OC_OR,   "   80 y.y. /1 y. sb " }, // immediate to reg/mem
    { OC_XOR,  "   80 y.y. /6 y. sb " }, // immediate to reg/mem
    { OC_SHL,  "   C0 y... /4 y. 08 " }, // shift mem/reg left by 8bit immediate
    { OC_SHR,  "   C0 y... /5 y. 08 " }, // shift mem/reg right by 8bit immediate

};
//   0     1     2     3     4     5     6     7     8     9     A     B     C     D     E     F
// 0000  0001  0010  0011  0100  0101  0110  0111  1000  1001  1010  1011  1100  1101  1110  1111  


bool load_encoding_info(asm_instruction *inst, struct encoding_info *info) {
    bool needs_immediate = inst->regimm_operand.is_immediate;
    bool needs_displacement = inst->regmem_operand.is_mem_addr_by_symbol;
    
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
        info->needs_modregrm                  = IS_TRUE(entry->table_str, NEEDS_MODREGRM_POS);
        info->displacement_without_modrm      = IS_TRUE(entry->table_str, HAS_DISPLACEMENT_POS);

        if (memcmp(&entry->table_str[IMMEDIATE_SUPPORT], "sb", 2) == 0)
            info->immediate_support = IMM_SIGN_EXP_BIT;
        else if (memcmp(&entry->table_str[IMMEDIATE_SUPPORT], "32", 2) == 0)
            info->immediate_support = IMM_FIXED32;
        else if (memcmp(&entry->table_str[IMMEDIATE_SUPPORT], "08", 2) == 0)
            info->immediate_support = IMM_FIXED8;
        else
            info->immediate_support = IMM_NONE;

        if (needs_immediate && info->immediate_support == IMM_NONE)
            continue;
        if (needs_displacement && !(info->needs_modregrm || info->displacement_without_modrm))
            continue;
        
        if (inst->operation == OC_CALL) {
            // CALL is different when we call a displacement or through a register.
            if (inst->regmem_operand.is_mem_addr_by_symbol && !info->displacement_without_modrm)
                continue;
            
            // through a pointer or address pointed by a register
            if (!inst->regmem_operand.is_mem_addr_by_symbol && info->displacement_without_modrm)
                continue;
        }

        return true;
    }

    return false;
}

