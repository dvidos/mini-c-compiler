#include <string.h>
#include "encoder4.h"
#include "asm_line.h"
#include "encoding_info.h"
#include "encoded_instruction.h"




static bool encode_asm_instr_opcode(asm_instruction *oper, struct encoding_info *info, struct encoded_instruction *result) {

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
        if (oper->direction_rm_to_ri_operands)  // one means from R/M --> Reg in the ModRegR/M byte                
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
    if (info->has_opcode_extension) {
        result->flags.have_modregrm = true;
        result->modregrm_byte |= ((info->opcode_extension_value & 0x7) << 3);
    }

    return true;
}

static bool encode_asm_instr_operands(asm_instruction *oper, struct encoding_info *info, struct encoded_instruction *result) {

    // see if we need mod/rm
    if (!info->needs_modregrm)
        return true;
    
    result->flags.have_modregrm = true;
    
    // first operand one
    if (oper->regmem_operand.is_register)
    {
        // easy set Mod to "11" and "R/M" to the register
        result->modregrm_byte |= (0x3 << 6);
        result->modregrm_byte |= (oper->regmem_operand.per_type.reg & 0x7);
    }
    else if (oper->regmem_operand.is_mem_addr_by_symbol)
    {
        // special case, set mod to '00' and r/m to '101'
        result->modregrm_byte |= (0x0 << 6);
        result->modregrm_byte |= (0x5);

        // fixed size 32-bits displacement in this case
        *(long *)result->displacement = oper->regmem_operand.per_type.mem.displacement;
        result->displacement_bytes_count = 4;
    }
    else if (oper->regmem_operand.is_memory_by_reg)
    {
        // see if we have an array notation (item must be 1,2,4 or 8)
        if (oper->regmem_operand.per_type.mem.array_item_size == 0)
        {
            // it's invalid to use SP as pointer, the notation is used for the SIB
            if (oper->regmem_operand.per_type.mem.pointer_reg == REG_SP)
                return false;
            
            // no SIB byte, proceed normally
            // "Mod" part will be set by the displacement size
            result->modregrm_byte |= oper->regmem_operand.per_type.mem.pointer_reg;
        }
        else // we have item size, use SIB
        {
            // flag SIB presence in Reg part of ModRegRM (32bits mode)
            result->modregrm_byte |= (0x4); // '100' signals SIB presense
            result->flags.have_sib = true;

            // scale x1, x2, x4, x8
            switch (oper->regmem_operand.per_type.mem.array_item_size) {
                case 1: result->sib_byte |= (0x0 << 6); break;
                case 2: result->sib_byte |= (0x1 << 6); break;
                case 4: result->sib_byte |= (0x2 << 6); break;
                case 8: result->sib_byte |= (0x3 << 6); break;
                default: return false;
            }
            // register to use for array index
            result->sib_byte |= ((oper->regmem_operand.per_type.mem.array_index_reg & 0x7) << 3);
            // register to use for base
            result->sib_byte |= (oper->regmem_operand.per_type.mem.pointer_reg & 0x7);
        }
        
        // SIB or not, add possible displacement
        if (oper->regmem_operand.per_type.mem.displacement == 0) {
            // no displacement, set mod to 00
            result->modregrm_byte |= (0x0 << 6);
        } else if (oper->regmem_operand.per_type.mem.displacement >= -128 && oper->regmem_operand.per_type.mem.displacement <= 127) {
            // one byte signed displacement follows
            result->modregrm_byte |= (0x1 << 6);
            result->displacement[0] = (char)oper->regmem_operand.per_type.mem.displacement;
            result->displacement_bytes_count = 1;
        } else {
            // full four bytes signed displacement follows
            result->modregrm_byte |= (0x2 << 6);
            *(long *)result->displacement = oper->regmem_operand.per_type.mem.displacement;
            result->displacement_bytes_count = 4;
        }
    } 

    // then, operand two (if it's immediate, see other function)
    if (oper->regimm_operand.is_register) {
        result->modregrm_byte |= ((oper->regimm_operand.per_type.reg & 0x7) << 3);
    }

    return true;
}

static bool encode_asm_instr_immediate(asm_instruction *oper, struct encoding_info *info, struct encoded_instruction *result) {

    // maybe we don't need anything
    if (!oper->regimm_operand.is_immediate)
        return true;
    
    // verify the instruction chosen supports immediate values
    if (!info->supports_immediate_value)
        return false;
    
    // set the full immediate value
    *(long *)result->immediate = (long)oper->regimm_operand.per_type.immediate;
    result->immediate_bytes_count = 4;

    // see if we can shorten the bytes added
    // "sign_expanded_immediate" takes the place of direction
    // 0=immediate as indicated by size bit (8/32 bits),
    // 1=immediate is 1-byte signed number, to be sign extended
    if (info->has_sign_expanded_immediate_bit 
        && oper->regimm_operand.per_type.immediate >= -128
        && oper->regimm_operand.per_type.immediate <= 127) {
            // set the sign expand bit and add just one immediate byte
            result->opcode_byte |= 0x2;
            *(char *)result->immediate = (char)oper->regimm_operand.per_type.immediate;
            result->immediate_bytes_count = 1;
    }

    return true;
}

bool encode_asm_instruction(asm_instruction *oper, struct encoding_info *info, struct encoded_instruction *result) {
    // logic in this function based largely on this page;
    // http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    // maybe good example will be:
    // "MOV [BP+CX*4+16], -16", it contains 16bit prefix, ModRegRM, SIB, displacement, immediate.
    // it should become:  op  rm  sid  offset  immediate
    //                    C7  44  8D   10      F0 FF FF FF

    memset(result, 0, sizeof(struct encoded_instruction));
    if (!encode_asm_instr_opcode(oper, info, result))
        return false;
    if (!encode_asm_instr_operands(oper, info, result))
        return false;
    if (!encode_asm_instr_immediate(oper, info, result))
        return false;
    
    return true;
}

