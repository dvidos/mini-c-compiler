#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "../../err_handler.h"
#include "../../utils/all.h"
#include "../../linker/obj_code.h"
#include "../asm_listing.h"
#include "encoder.h"
#include "encoded_instruction.h"
#include "encoding_info.h"

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;



static bool _encode_v4(x86_encoder *encoder, asm_instruction *instr);
static void _reset(x86_encoder *enc);
static void _free(x86_encoder *enc);

static bool encode_asm_instruction(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result);


struct x86_encoder *new_x86_encoder(mempool *mp, bin *code_out, reloc_list *relocations_out) {
    struct x86_encoder *enc = mpalloc(mp, struct x86_encoder);

    enc->output = code_out;
    enc->relocations = relocations_out;
    enc->mempool = mp;

    enc->encode_v4 = _encode_v4;
    enc->reset = _reset;
    enc->free = _free;

    return enc;
};



static bool _encode_v4(x86_encoder *encoder, asm_instruction *instr) {
    struct encoding_info enc_info;
    mempool *mp = new_mempool();
    str *s;

    if (!load_encoding_info(instr, &enc_info)) {
        error("Failed loading encoding info for operation '%s'\n", instr_code_name(instr->operation));
        return false;
    }

    encoded_instruction enc_instr;
    if (!encode_asm_instruction(instr, &enc_info, &enc_instr)) {
        error("Failed encoding instruction: '%s'\n", instr_code_name(instr->operation));
        mempool_release(mp);
        return false;
    }

    // we have not solved for rellocations or symbol tables yet
    int old_index = bin_len(encoder->output);
    pack_encoded_instruction(&enc_instr, encoder->output);

    // show the conversion
    // s = new_str(mp, NULL);
    // asm_instruction_to_str(instr, s, false);
    // printf("%-20s >> ", str_charptr(s));
    // str_clear(s);
    // encoded_instruction_to_str(&enc_instr, s);
    // printf("%s >> ", str_charptr(s));
    // str_clear(s);
    // bin_print_hex(encoder->output, 0, 0, -1, stdout);
    // printf("\n");

    return true;
}

static void _reset(struct x86_encoder *enc) {
    bin_clear(enc->output);
    enc->relocations->clear(enc->relocations);
}

static void _free(struct x86_encoder *enc) {
    enc->relocations->free(enc->relocations);
    free(enc);
}

// -----------------------------
// encoder v4 below
// -----------------------------


static bool encode_asm_instr_opcode(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // setup 16-bit operand size if needed
    if (instr->operands_size_bits == 16) {
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
        if (instr->direction_op1_to_op2)  // one means from R/M --> Reg in the ModRegR/M byte                
            result->opcode_byte |= 0x2;
        else // zero means from Reg --> R/M in the ModRegR/M byte
            result->opcode_byte &= ~0x2;
    }

    // width bit works both for immediate and for reg-mem types of instructions
    if (info->has_width_bit) {
        if (instr->operands_size_bits == 8) // zero indicates single byte operands
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

static bool encode_asm_instr_operands(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // most instructions need mod/rm byte
    if (info->needs_modregrm || info->has_opcode_extension) {
        result->flags.have_modregrm = true;
        
        // first operand one
        if (instr->operand1.is_register)
        {
            // easy set Mod to "11" and "R/M" to the register
            result->modregrm_byte |= (0x3 << 6);
            result->modregrm_byte |= (instr->operand1.per_type.reg & 0x7);
        }
        else if (instr->operand1.is_mem_addr_by_symbol)
        {
            // special case, set mod to '00' and r/m to '101'
            result->modregrm_byte |= (0x0 << 6);
            result->modregrm_byte |= (0x5);

            // fixed size 32-bits displacement in this case
            *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
            result->displacement_bytes_count = 4;
        }
        else if (instr->operand1.is_memory_by_reg)
        {
            // see if we have an array notation (item must be 1,2,4 or 8)
            if (instr->operand1.per_type.mem.array_item_size == 0)
            {
                // it's invalid to use SP as pointer, the notation is used for the SIB
                if (instr->operand1.per_type.mem.pointer_reg == REG_SP)
                    return false;
                
                // no SIB byte, proceed normally
                // "Mod" part will be set by the displacement size
                result->modregrm_byte |= instr->operand1.per_type.mem.pointer_reg;
            }
            else // we have item size, use SIB
            {
                // flag SIB presence in Reg part of ModRegRM (32bits mode)
                result->modregrm_byte |= (0x4); // '100' signals SIB presense
                result->flags.have_sib = true;

                // scale x1, x2, x4, x8
                switch (instr->operand1.per_type.mem.array_item_size) {
                    case 1: result->sib_byte |= (0x0 << 6); break;
                    case 2: result->sib_byte |= (0x1 << 6); break;
                    case 4: result->sib_byte |= (0x2 << 6); break;
                    case 8: result->sib_byte |= (0x3 << 6); break;
                    default: return false;
                }
                // register to use for array index
                result->sib_byte |= ((instr->operand1.per_type.mem.array_index_reg & 0x7) << 3);
                // register to use for base
                result->sib_byte |= (instr->operand1.per_type.mem.pointer_reg & 0x7);
            }
            
            // SIB or not, add possible displacement
            if (instr->operand1.per_type.mem.displacement == 0) {
                // no displacement, set mod to 00
                result->modregrm_byte |= (0x0 << 6);
            } else if (instr->operand1.per_type.mem.displacement >= -128 && instr->operand1.per_type.mem.displacement <= 127) {
                // one byte signed displacement follows
                result->modregrm_byte |= (0x1 << 6);
                result->displacement[0] = (char)instr->operand1.per_type.mem.displacement;
                result->displacement_bytes_count = 1;
            } else {
                // full four bytes signed displacement follows
                result->modregrm_byte |= (0x2 << 6);
                *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
                result->displacement_bytes_count = 4;
            }
        } 

        // then, operand two (if it's immediate, see other function)
        if (info->has_opcode_extension) {
            result->modregrm_byte |= ((info->opcode_extension_value & 0x7) << 3);
        }
        else if (instr->operand2.is_register) {
            result->modregrm_byte |= ((instr->operand2.per_type.reg & 0x7) << 3);
        }
    }

    // some instructions take a displacement without modregrm byte (e.g. JMP)
    if (instr->operand1.is_mem_addr_by_symbol && info->displacement_without_modrm && !info->needs_modregrm) {
        // must save relocation position!!!!!!
        *(long *)result->displacement = instr->operand1.per_type.mem.displacement;
        result->displacement_bytes_count = 4;
        return true;
    }
    
    return true;
}

static bool encode_asm_instr_immediate(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {

    // maybe we don't need anything
    if (!instr->operand2.is_immediate)
        return true;
    
    // verify the instruction chosen supports immediate values
    if (info->immediate_support == IMM_NONE)
        return false;
    
    bool fits_single_byte = instr->operand2.per_type.immediate >= -128 && instr->operand2.per_type.immediate <= 127;

    if (info->immediate_support == IMM_FIXED8) {
        *(char *)result->immediate = (char)(instr->operand2.per_type.immediate & 0xFF);
        result->immediate_bytes_count = 1;
    } else if (info->immediate_support == IMM_FIXED32) {
        *(long *)result->immediate = (long)instr->operand2.per_type.immediate;
        result->immediate_bytes_count = 4;
    } else if (info->immediate_support == IMM_SIGN_EXP_BIT) {
        if (fits_single_byte) {
            // set the sign expand bit and add just one immediate byte
            // "sign_expanded_immediate" takes the place of direction bit
            // 0=immediate as indicated by size bit (8/32 bits),
            // 1=immediate is 1-byte signed number, to be sign extended
            result->opcode_byte |= 0x2;
            *(char *)result->immediate = (char)(instr->operand2.per_type.immediate & 0xFF);
            result->immediate_bytes_count = 1;
        } else {
            *(long *)result->immediate = (long)instr->operand2.per_type.immediate;
            result->immediate_bytes_count = 4;
        }
    }

    return true;
}

static bool encode_asm_instruction(asm_instruction *instr, struct encoding_info *info, struct encoded_instruction *result) {
    // logic in this function based largely on this page;
    // http://www.c-jump.com/CIS77/CPU/x86/lecture.html

    // maybe good example will be:
    // "MOV [BP+CX*4+16], -16", it contains 16bit prefix, ModRegRM, SIB, displacement, immediate.
    // it should become:  C7  44  8D   10    F0 FF FF FF
    //                    op  rm  sid  offs   immediate

    memset(result, 0, sizeof(struct encoded_instruction));
    if (!encode_asm_instr_opcode(instr, info, result))
        return false;
    if (!encode_asm_instr_operands(instr, info, result))
        return false;
    if (!encode_asm_instr_immediate(instr, info, result))
        return false;
    return true;
}

