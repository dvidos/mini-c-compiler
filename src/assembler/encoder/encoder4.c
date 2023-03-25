#include <string.h>
#include "encoder4.h"
#include "asm_instruction.h"


static bool encode_opcode(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);
static bool encode_operands(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);
static bool encode_immediate(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result);


bool encode_cpu_operation(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {
    // logic in this function based largely on this page;
    // http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    // maybe good example will be:
    // "MOV [BP+CX*4+16], -16", it contains 16bit prefix, ModRegRM, SIB, displacement, immediate.

    memset(result, 0, sizeof(struct encoded_instruction));
    if (!encode_opcode(oper, info, result))
        return false;
    if (!encode_operands(oper, info, result))
        return false;
    if (!encode_immediate(oper, info, result))
        return false;
    return true;
}

static bool encode_opcode(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {

    // setup 16-bit operand size if needed
    if (oper->operands_size_bits == 16) {
        // not sure of difference between operands size and address size...
        result->flags.have_operand_size_prefix = 1;
        result->operand_size_prefix = 0x66;
    }

    // find if instruction requires expansion byte and set it up.
    if (info->has_instruction_expansion_byte) {
        // some instructions have a 0x0F (e.g. BSF)
        result->flags.have_opcode_expansion_byte = 1;
        result->opcode_expansion_byte = info->instruction_expansion_byte;
    }

    result->opcode_byte = info->base_opcode_byte;

    if (info->has_direction_bit) {
        if (oper->direction_op1_to_op2)  // one means from R/M --> Reg in the ModRegR/M byte                
            result->opcode_byte |= 0x2;
        else // zero means from Reg --> R/M in the ModRegR/M byte
            result->opcode_byte &= ~0x2;
    }

    // width bit works both for immediate and for reg-mem types of instructions
    if (info->has_width_bit) {
        if (oper->operands_size_bits == 8) // zero indicates single byte operands
            result->opcode_byte &= ~0x1;
        else // one indicates full size operands
            result->opcode_byte |= 0x1;
    }
    
    // some immediate instructions have this (e.g. "SAL D0 /7")
    if (info->opcode_extension != -1) {
        result->flags.have_modregrm = true;
        result->modregrm_byte |= ((info->opcode_extension & 0x7) << 3);
    }
}

static bool encode_operands(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {

    // see if we need mod/rm
    if (!info->needs_modregrm)
        return true;
    
    result->flags.have_modregrm = true;
    
    // first operand one
    if (oper->operand1.is_register)
    {
        // easy set Mod to "11" and "R/M" to the register
        result->modregrm_byte |= (0x3 << 6);
        result->modregrm_byte |= (oper->operand1.per_type.reg & 0x7);
    }
    else if (oper->operand1.is_memory_by_displacement)
    {
        // special case, set mod to '00' and r/m to '101'
        result->modregrm_byte |= (0x0 << 6);
        result->modregrm_byte |= (0x5);

        // fixed size 32-bits displacement in this case
        *(long *)result->displacement = oper->operand1.per_type.mem.displacement;
        result->displacement_bytes_count = 4;
    }
    else if (oper->operand1.is_memory_by_reg)
    {
        // see if we have an array notation (item must be 1,2,4 or 8)
        if (oper->operand1.per_type.mem.array_item_size == 0)
        {
            // it's invalid to use SP as pointer, the notation is used for the SIB
            if (oper->operand1.per_type.mem.pointer_reg == REG_SP)
                return false;
            
            // no SIB byte, proceed normally
            // "Mod" part will be set by the displacement size
            result->modregrm_byte |= oper->operand1.per_type.mem.pointer_reg;
        }
        else // we have item size, use SIB
        {
            // flag SIB presence in Reg part of ModRegRM (32bits mode)
            result->modregrm_byte |= (0x4); // '100' signals SIB presense
            result->flags.have_sib = true;

            // scale x1, x2, x4, x8
            switch (oper->operand1.per_type.mem.array_item_size) {
                case 1: result->sib_byte |= (0x0 << 6); break;
                case 2: result->sib_byte |= (0x1 << 6); break;
                case 4: result->sib_byte |= (0x2 << 6); break;
                case 8: result->sib_byte |= (0x3 << 6); break;
                default: return false;
            }
            // register to use for array index
            result->sib_byte |= ((oper->operand1.per_type.mem.array_index_reg & 0x7) << 3);
            // register to use for base
            result->sib_byte |= (oper->operand1.per_type.mem.pointer_reg & 0x7);
        }
        
        // SIB or not, add possible displacement
        if (oper->operand1.per_type.mem.displacement == 0) {
            // no displacement, set mod to 00
            result->modregrm_byte |= (0x0 << 6);
        } else if (oper->operand1.per_type.mem.displacement >= -128 && oper->operand1.per_type.mem.displacement <= 127) {
            // one byte signed displacement follows
            result->modregrm_byte |= (0x1 << 6);
            result->displacement[0] = (char)oper->operand1.per_type.mem.displacement;
            result->displacement_bytes_count = 1;
        } else {
            // full four bytes signed displacement follows
            result->modregrm_byte |= (0x2 << 6);
            *(long *)result->displacement = oper->operand1.per_type.mem.displacement;
            result->displacement_bytes_count = 4;
        }
    } 

    // then, operand two (if it's immediate, see other function)
    if (oper->operand2.is_register) {
        result->modregrm_byte |= ((oper->operand2.per_type.reg & 0x7) << 3);
    }

    return true;
}

static bool encode_immediate(struct asm_operation *oper, struct encoding_info *info, struct encoded_instruction *result) {

    // maybe we don't need anything
    if (!oper->operand2.is_immediate)
        return true;
    
    // verify the instruction chosen supports immediate values
    if (!info->supports_immediate_value)
        return false;
    
    // set the full immediate value
    *(long *)result->immediate = (long)oper->operand2.per_type.immediate;
    result->immediate_bytes_count = 1;

    // see if we can shorten the bytes added
    // "sign_expanded_immediate" takes the place of direction
    // 0=immediate as indicated by size bit (8/32 bits),
    // 1=immediate is 1-byte signed number, to be sign extended
    if (info->has_sign_expanded_immediate_bit 
        && oper->operand2.per_type.immediate >= -128
        && oper->operand2.per_type.immediate <= 127) {
            // set the sign expand bit and add just one immediate byte
            result->opcode_byte |= 0x2;
            *(char *)result->immediate = (char)oper->operand2.per_type.immediate;
            result->immediate_bytes_count = 1;
    }

    return true;
}


void pack_encoded_instruction(struct encoded_instruction *inst, char *buffer, int *buffer_length) {

    // prefixes
    if (inst->flags.have_instruction_prefix) {
        *buffer++ = inst->instruction_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_address_size_prefix) {
        *buffer++ = inst->address_size_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_operand_size_prefix) {
        *buffer++ = inst->operand_size_prefix;
        (*buffer_length)++;
    }
    if (inst->flags.have_segment_override_prefix) {
        *buffer++ = inst->segment_override_prefix;
        (*buffer_length)++;
    }

    // opcode(s)
    if (inst->flags.have_opcode_expansion_byte) {
        *buffer++ = inst->opcode_expansion_byte;
        (*buffer_length)++;
    }
    *buffer++ = inst->opcode_byte;
    (*buffer_length)++;

    // operands
    if (inst->flags.have_modregrm) {
        *buffer++ = inst->modregrm_byte;
        (*buffer_length)++;
    }
    if (inst->flags.have_sib) {
        *buffer++ = inst->sib_byte;
        (*buffer_length)++;
    }

    // displacement & immediate
    if (inst->displacement_bytes_count > 0) {
        memcpy(buffer, inst->displacement, inst->displacement_bytes_count);
        buffer += inst->displacement_bytes_count;
        (*buffer_length) += inst->displacement_bytes_count;
    }
    if (inst->immediate_bytes_count > 0) {
        memcpy(buffer, inst->immediate, inst->immediate_bytes_count);
        buffer += inst->immediate_bytes_count;
        (*buffer_length) += inst->immediate_bytes_count;
    }
}

void print_asm_operation(struct asm_operation *oper, FILE *stream) {
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

void print_encoded_instruction(struct encoded_instruction *inst, FILE *stream) {
/*
    4 prefix bytes, 2 opcode bytes, modregrm, sib, 0-4 displacement, 0-4 immediate

    "Ins Adr Opr Seg   Opc         ModR/M       SIB                                "
    "Pfx Siz Siz Ovr   Pfx Opc   Md Reg R/M  SS Idx Bse   Displacemnt   Immediate  "
    " 00  --  00  --    00  00   00.000.000  00.000.000   00 00 00 00   00 00 -- --"
*/
    char num[16];
    char buffer[128];

    if (inst == NULL) {
        fprintf(stream, "Ins Adr Opr Seg   Opc         ModR/M       SIB                                \n");
        fprintf(stream, "Pfx Siz Siz Ovr   Pfx Opc   Md Reg R/M  SS Idx Bse   Displacemnt   Immediate  \n");
        return;
    }
    //                   0         1         2         3         4         5         6         7         8
    //                   012345678901234567890123456789012345678901234567890123456789012345678901234567890
    strcpy(buffer,      " --  --  --  --    --  --   --.---.---  --.---.---   -- -- -- --   -- -- -- --");

    if (inst->flags.have_instruction_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->instruction_prefix);
        memcpy(buffer + 1, num, 2);
    }
    if (inst->flags.have_address_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->address_size_prefix);
        memcpy(buffer + 5, num, 2);
    }
    if (inst->flags.have_operand_size_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->operand_size_prefix);
        memcpy(buffer + 9, num, 2);
    }
    if (inst->flags.have_segment_override_prefix) {
        sprintf(num, "%02x", (unsigned char)inst->segment_override_prefix);
        memcpy(buffer + 13, num, 2);
    }
    if (inst->flags.have_opcode_expansion_byte) {
        sprintf(num, "%02x", (unsigned char)inst->opcode_expansion_byte);
        memcpy(buffer + 19, num, 2);
    }
    sprintf(num, "%02x", (unsigned char)inst->opcode_byte);
    memcpy(buffer + 23, num, 2);

    if (inst->flags.have_modregrm) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->modregrm_byte >> 7) & 0x1,
            (inst->modregrm_byte >> 6) & 0x1,
            (inst->modregrm_byte >> 5) & 0x1,
            (inst->modregrm_byte >> 4) & 0x1,
            (inst->modregrm_byte >> 3) & 0x1,
            (inst->modregrm_byte >> 2) & 0x1,
            (inst->modregrm_byte >> 1) & 0x1,
            (inst->modregrm_byte >> 0) & 0x1);
        memcpy(buffer + 28, num, 10);
    }
    if (inst->flags.have_sib) {
        sprintf(num, "%d%d.%d%d%d.%d%d%d", 
            (inst->sib_byte >> 7) & 0x1,
            (inst->sib_byte >> 6) & 0x1,
            (inst->sib_byte >> 5) & 0x1,
            (inst->sib_byte >> 4) & 0x1,
            (inst->sib_byte >> 3) & 0x1,
            (inst->sib_byte >> 2) & 0x1,
            (inst->sib_byte >> 1) & 0x1,
            (inst->sib_byte >> 0) & 0x1);
        memcpy(buffer + 40, num, 10);
    }
    if (inst->displacement_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[0]);
        memcpy(buffer + 53, num, 2);
    }
    if (inst->displacement_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[1]);
        memcpy(buffer + 56, num, 2);
    }
    if (inst->displacement_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[2]);
        memcpy(buffer + 59, num, 2);
    }
    if (inst->displacement_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->displacement[3]);
        memcpy(buffer + 62, num, 2);
    }
    if (inst->immediate_bytes_count > 0) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[0]);
        memcpy(buffer + 67, num, 2);
    }
    if (inst->immediate_bytes_count > 1) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[1]);
        memcpy(buffer + 70, num, 2);
    }
    if (inst->immediate_bytes_count > 2) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[2]);
        memcpy(buffer + 73, num, 2);
    }
    if (inst->immediate_bytes_count > 3) {
        sprintf(num, "%02x", (unsigned char)inst->immediate[3]);
        memcpy(buffer + 76, num, 2);
    }
    fprintf(stream, "%s\n", buffer);
}
