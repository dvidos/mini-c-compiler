#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "assembler.h"
#include "../err_handler.h"
#include "../run_info.h"
#include "../compiler/codegen/ir_listing.h"
#include "../linker/obj_code.h"
#include "encoder/encoding_info.h"
#include "encoder/encoded_instruction.h"
#include "encoder/encoder.h"
#include "encoder/asm_allocator.h"
#include "asm_line.h"
#include "asm_listing.h"
#include "../utils/all.h"
#include "../elf/obj_module.h"


typedef struct assembler_data assembler_data;

struct assembler_data {
    mempool *mempool;
    
    // for the current module getting assembled.
    asm_listing *asm_listing;
    list *globals; // item type is "str"
    list *externs; // item type is "str"
    obj_module *module;
    obj_section *curr_sect;
};

static obj_module *assemble_listing_into_x86_64_code(assembler *as, asm_listing *asm_list, str *module_name);

static struct assembler_ops ops = {
    .assemble_listing_into_x86_64_code = assemble_listing_into_x86_64_code,
};

assembler *new_assembler(mempool *mp) {
    assembler_data *ad = mpalloc(mp, assembler_data);
    ad->mempool = mp;

    assembler *as = mpalloc(mp, assembler);
    as->ops = &ops;
    as->priv_data = ad;
    return as;
}

void assemble_listing_into_i386_code(mempool *mp, asm_listing *asm_list, obj_code *obj) {

    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(mp, obj->text->contents, obj->text->relocations);
    asm_instruction *inst;
    struct encoding_info enc_info;
    encoded_instruction enc_inst;

    for_list(asm_list->lines, asm_line, line) {
        if (line->type != ALT_INSTRUCTION)
            continue;
        
        if (line->label != NULL) {
            // we don't know if this is exported for now
            obj->text->symbols->add(obj->text->symbols, str_charptr(line->label), 
                bin_len(obj->text->contents), 0, ST_FUNCTION, false);
        }
        
        inst = line->per_type.instruction;
        if (inst->operation == OC_NONE)
            continue;
        
        if (!enc->encode_v4(enc, inst)) {
            error("Failed encoding instruction: '%s'\n", str_charptr(asm_line_to_str(mp, line)));
            continue;
        }
    }

    printf("Sample resulting machine code\n");
    bin_print_hex(obj->text->contents, 0, 0, -1, stdout);
    printf("\n");
}


// ------------------------------------------------

int compare_str_with_charptr(const void *a, const void *b) {
    str *s_ptr = (str *)a;
    char *char_ptr = (char *)b;
    return strcmp(str_charptr(s_ptr), char_ptr) == 0;
}
static int compare_str(const void *a, const void *b) {
    return str_cmp((str *)a, (str *)b);
}
static inline u8 mod_reg_rm(u8 mod, u8 reg, u8 rm) {
    return ((mod & 0x3) << 6) | ((reg & 0x7) << 3) | (rm & 0x7);
}
static inline u8 rex_byte_without_sib(u8 w_bit, u8 reg_bit3, u8 rm_bit3) {
    return 0x40 | ((w_bit & 1) << 3) | ((reg_bit3 & 1) << 2) | (rm_bit3 & 1);
}
static inline u8 rex_byte_with_sib(u8 w_bit, u8 reg_bit3, u8 index_bit3, u8 base_bit3) {
    // when SIB is present, rm part of modregrm is 100
    // so the rm bit is used for SIB.base
    return 0x40 | ((w_bit & 1) << 3) | ((reg_bit3 & 1) << 2) | ((index_bit3 & 1) << 1) | (base_bit3 & 1);
}

enum modregrm_addressing_mode {
    // values for the "mod" part of the mod-reg-rm
    MOD_00_INDIRECT_MEM_NO_DISPLACEMENT = 0,
    MOD_01_INDIRECT_MEM_ONE_BYTE_DISPL = 1,
    MOD_10_INDIRECT_MEM_FOUR_BYTES_DISPL = 2,
    MOD_11_DIRECT_REGISTER = 3
};

enum modregrm_register {
    // values for the "reg" part of the mod-reg-rm
    // REX.R=1 makes these in the range of r8..r15
    AX = 0, CX = 1, DX = 2, BX = 3, SP = 4, BP = 5, SI = 6, DI = 7
};

enum modregrm_rm {
    // values for the "rm" part of the mod-reg-rm
    // REX.B=1 makes these in the range of r8..r15
    RM_000_AX = 0, 
    RM_001_CX = 1, 
    RM_010_DX = 2, 
    RM_011_BX = 3,
    RM_100_SIB_IF_MOD_NOT_11 = 4, 
    RM_100_SP_IF_MOD_11 = 4,
    RM_101_DISPL32_IF_MOD_00 = 5,
    RM_101_BP_IF_MOD_NOT_00 = 5,
    RM_110_SI = 6, 
    RM_111_DI = 7
};

static int get_register_bits(gp_register r) {
    if ((r >= REG_RAX && r <= REG_RDI) || (r >= REG_R8  && r <= REG_R15))  return 64;
    if ((r >= REG_EAX && r <= REG_EDI) || (r >= REG_R8D && r <= REG_R15D)) return 32;
    if ((r >= REG_AX  && r <= REG_DI)  || (r >= REG_R8W && r <= REG_R15W)) return 16;
    if ((r >= REG_AL  && r <= REG_BH)  || (r >= REG_R8B && r <= REG_R15B)) return 8;
    return 0;
}

static bool get_instruction_data_size(asm_instruction *instr, u8 *bits) {

    // reg <- reg, grab from reg, ensure same sizes
    // mem <- reg, grab size from src reg
    // reg <- mem, grab size from dest reg
    // reg <- imm, grab size from dest reg
    // mem <- imm, size must be explicit
    //
    // notice that a register used for pointing is ignored here,
    // because the size of this pointer is irrelevant to the data size.

    if (instr->regmem_operand.is_register && instr->regimm_operand.is_register) {
        // reg <--> reg
        u8 s1 = get_register_bits(instr->regmem_operand.per_type.reg);
        u8 s2 = get_register_bits(instr->regimm_operand.per_type.reg);
        if (s1 != s2) {
            error("reg <-> reg operations must have the same size");
            return false;
        }
        *bits = s1;
    } else if ((instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) &&
                instr->regimm_operand.is_register) {
        // memory <--> register
        *bits = get_register_bits(instr->regimm_operand.per_type.reg);
    } else if (instr->regmem_operand.is_register && instr->regimm_operand.is_immediate) {
        // register <-- immediate
        *bits = get_register_bits(instr->regmem_operand.per_type.reg);
    } else if ((instr->regmem_operand.is_memory_by_reg || instr->regmem_operand.is_mem_addr_by_symbol) && 
                instr->regimm_operand.is_immediate) {
        // immediate to memory, we must have explicit memory size
        if (instr->operands_size_bits < 8 || instr->operands_size_bits > 64) {
            error("mem <-- imm operations must have explicit size definition");
            return false;
        }
        *bits = instr->operands_size_bits;
    } else {
        fatal("Unexpected operands configuration!");
        return false;
    }

    return true;
}

static bool is_8bits_instruction(asm_instruction *instr) {
    u8 bits;
    if (!get_instruction_data_size(instr, &bits))
        return false;
    
    return (bits == 8);
}

static bool is_instruction_with_immediate(asm_instruction *instr) {
    return instr->regimm_operand.is_immediate;
}

static bool is_valid_data_size_instruction(asm_instruction *instr) {
    u8 bits;
    if (!get_instruction_data_size(instr, &bits))
        return false;
    
    return (bits == 8 || bits == 16 || bits == 32 || bits == 64);
}

static bool get_instruction_pointer_size(asm_instruction *instr, u8 *bits) {
    if (instr->regmem_operand.is_memory_by_reg) {
        *bits = get_register_bits(instr->regmem_operand.per_type.reg);
    } else {
        *bits = 0; // no pointer
    }
    return true;
}

static bool encode_size_prefix_bytes(asm_instruction *instr, 
    int opcode_8bits, int opcode_16plus, 
    bool *need_66, bool *need_67, bool *need_rex,
    u8 *rex_value, u8 *opcode_value
) {
    u8 data_bits = 0;
    u8 ptr_bits = 0;
    if (!get_instruction_data_size(instr, &data_bits))
        return false;
    if (!get_instruction_pointer_size(instr, &ptr_bits))
        return false;

    /* Regarding encoding:
        - data size, for how many bytes to write
            - for reg-reg, they must be equal
            - for reg-imm, the register size is used
            - for reg-mem / mem-reg, the register size is used
            - for mem-imm, it must be explicitely defined
        - address size is the pointer size, if we are using indirect memory address through register.
            - e.g. MOV DX, [ESI]  needs 0x67
            -      MOV DX, [RSI]  does not need 0x67
        For encoding the x86_64 instructions:
            - width bit of the opcode for data size 8 bits
            - 66 wherever data size is 16 bits
            - 32 bits operations is the unescaped default, even in x86_64
            - REX.W (48) wherever data size is 64 bits
            - 67 wherever pointer size  is 32 bits instead of 64.
    */

    *need_66 = false;
    *need_67 = false;
    *need_rex = false;

    if (data_bits == 8) {
        if (opcode_8bits < 0 || opcode_8bits > 0xFF) {
            error("instruction does not support 8 bits operation");
            return false;
        }
        *opcode_value = opcode_8bits;
    } else if (data_bits == 16) {
        *need_66 = true;
        *opcode_value = opcode_16plus;
    } else if (data_bits == 32) {
        *opcode_value = opcode_16plus;
    } else if (data_bits == 64) {
        *need_rex = true;
        *rex_value = 0x48;
        *opcode_value = opcode_16plus;
    }

    // irrespective of data size, 67 is only about address size
    if (ptr_bits == 32) {
        *need_67 = true;
    }

    return true;
}

static bool encode_addressing_bytes(asm_reg_or_mem_operand *oper, u8 modregrm_reg, 
    u8 *modregrm_value, 
    u32 *displacement32_value, 
    bool *need_displacement32, 
    bool *need_symbol_relocation
) {
    *need_displacement32 = false;
    *need_symbol_relocation = false;

    u8 mod;
    u8 rm;

    if (oper->is_register) {
        // direct register selection
        mod = MOD_11_DIRECT_REGISTER;
        rm = (oper->per_type.reg & 0x7);
        *need_displacement32 = false;
        *need_symbol_relocation = false;
        *modregrm_value = mod_reg_rm(mod, modregrm_reg, rm);

    } else {
        // this is memory access, using a pointer or address (via a symbol)
        // should verify the operand size, so see if we need any size prefixes.
        // in x86_64, the default size is 64 bits, use prefix 0x67 to switch to 32 bits.
        // seems that, if both operands are 64 bits (e.g. "mov rax, [rbx]")
        // we don't need the bit. but if it is immediate, we do need it...
        /*
        pointed memory:
        if  8 bits, we need to run off the size bit (bit 0)
        if 16 bits, bit 0 is on and prefix 66 is added
        if 32 bits, no prefix is needed
        if 64 bits, we need REX prefix (0x48)

        if we are moving between memory and an immediate,
        the size of the pointed memory MUST be defined.

        the important thing is the size of the pointed memory
        (e.g. char* vs long*)
        if Exx is used for pointer instead of Rxx, we just prefix with 0x67

        if there is a register involved, we can use the size 
        of the register to deduce operation size.
        */

        if (oper->is_memory_by_reg) { 
            // memory, pointed by register
            if (oper->per_type.mem.displacement != 0) {
                mod = MOD_10_INDIRECT_MEM_FOUR_BYTES_DISPL;
                rm = (oper->per_type.mem.pointer_reg & 0x7);
                *displacement32_value = oper->per_type.mem.displacement;
                *need_displacement32 = true;
            } else {
                mod = MOD_00_INDIRECT_MEM_NO_DISPLACEMENT;
                rm = (oper->per_type.mem.pointer_reg & 0x7);
            }
        } else if (oper->is_mem_addr_by_symbol) { 
            // memory, address of a symbol
            mod = 0x0; // 00 to enable special DISPL32 r/m mode
            rm = RM_101_DISPL32_IF_MOD_00;
            *displacement32_value = 0;
            *need_displacement32 = true;
            *need_symbol_relocation = true;
        }
    }

    *modregrm_value = mod_reg_rm(mod, modregrm_reg, rm);
    return true;
}

static bool encode_immediate_info(asm_instruction *instr, bool *need_immediate, u8 *immediate_bits) {
    u8 data_size;
    if (!get_instruction_data_size(instr, &data_size))
        return false;

    *need_immediate = instr->regimm_operand.is_immediate;
    // the way I understand it, immediates for x86_64 are still 32 bits.
    *immediate_bits = data_size > 32 ? 32 : data_size;
    return true;
}

static bool encode_two_operands_instruction(assembler_data *ad, asm_line *line,
    u8 opcode_8bits_reg, u8 opcode_16plus_reg,
    u8 opcode_8bits_imm, u8 opcode_16plus_imm,
    int opcode_extension_for_imm
) {
    bool need_66;
    bool need_67;
    bool need_rex;
    bool need_displacement32;
    bool need_symbol_relocation;
    bool need_immediate;
    u8 rex_value;
    u8 opcode_value;
    u8 modregrm_value;
    u32 displacement32_value;
    u8 immediate_bits;

    asm_instruction *inst = line->per_type.instruction;

    if (!is_valid_data_size_instruction(inst)) {
        error("Cannot establish good data size for instruction");
        return false;
    }

    u8 opcode_8bits  = is_instruction_with_immediate(inst) ?  opcode_8bits_imm : opcode_8bits_reg;
    u8 opcode_16plus = is_instruction_with_immediate(inst) ? opcode_16plus_imm : opcode_16plus_reg;
    u8 modregrm_reg  = is_instruction_with_immediate(inst) ? opcode_extension_for_imm : inst->regimm_operand.per_type.reg;

    if (!encode_size_prefix_bytes(inst, opcode_8bits, opcode_16plus,
            &need_66, &need_67, &need_rex, &rex_value, &opcode_value))
        return false;

    if (!encode_addressing_bytes(&inst->regmem_operand, modregrm_reg, 
            &modregrm_value, &displacement32_value, &need_displacement32, &need_symbol_relocation))
        return false;
    
    if (!encode_immediate_info(inst, &need_immediate, &immediate_bits))
        return false;

    // start adding bytes and relocation if needed
    if (need_67)
        bin_add_byte(ad->curr_sect->contents, 0x67);
    if (need_66)
        bin_add_byte(ad->curr_sect->contents, 0x66);
    if (need_rex)
        bin_add_byte(ad->curr_sect->contents, rex_value);
    
    bin_add_byte(ad->curr_sect->contents, opcode_value);
    bin_add_byte(ad->curr_sect->contents, modregrm_value);
    if (need_symbol_relocation) {
        str *name = new_str(ad->mempool, inst->regmem_operand.per_type.mem.displacement_symbol_name);
        size_t offs = bin_len(ad->curr_sect->contents);
        ad->curr_sect->ops->add_relocation(ad->curr_sect, offs, name, 0, 0);
    }
    if (need_displacement32) {
        bin_add_dword(ad->curr_sect->contents, displacement32_value);
    }
    if (need_immediate) {
        if (immediate_bits == 8)
            bin_add_byte(ad->curr_sect->contents, (u8)inst->regimm_operand.per_type.immediate);
        else if (immediate_bits == 16)
            bin_add_word(ad->curr_sect->contents, (u16)inst->regimm_operand.per_type.immediate);
        else if (immediate_bits == 32)
            bin_add_dword(ad->curr_sect->contents, (u32)inst->regimm_operand.per_type.immediate);
        else if (immediate_bits == 64)
            bin_add_qword(ad->curr_sect->contents, (u64)inst->regimm_operand.per_type.immediate);
    }
}


// assembles the instruction line into the current section of the current module.
static void encode_instruction_line_x86_64(assembler_data *ad, asm_line *line) {
    // ref: http://ref.x86asm.net/coder64-abc.html#M

    // register the symbol, if public
    if (line->label != NULL) {
        bool is_global = list_find_first(ad->globals, compare_str, line->label) != -1;
        ad->curr_sect->ops->add_symbol(ad->curr_sect, line->label, bin_len(ad->curr_sect->contents), 0, is_global);
    }

    asm_instruction *instr = line->per_type.instruction;
    switch (instr->operation) {
        case OC_RET:
            bin_add_byte(ad->curr_sect->contents, 0xC3); // 0xCB is "lret" (long ret)
            break;
        case OC_MOV:
            u8 opcode_8bits   = instr->regimm_operand.is_immediate ? 0xC6 : 0x88;
            u8 opcode_16plius = instr->regimm_operand.is_immediate ? 0xC7 : 0x89;
            /*  88/r mov rm8 <- r8
                89/r mov rm16/32/64 <- r16/32/64
                8A/r mov rm8 -> r8
                8B/r mov rm16/32/64 -> r16/32/64
                B0+r mov r8 <- imm8
                B8+r mov r16/32/64 <- imm16/32/64
                C6/0 mov rm8 <- imm8
                C7/0 mov rm16/32/64 <- imm16/32/64 */
            encode_two_operands_instruction(ad, line, 0x88, 0x89, 0xC6, 0xC7, 0);
            break;
        default:
            error("Unsupported assembly instruction '%s'", instr_code_name(instr->operation));
            break;
    }

    // encode into bytes and add relocation. what can go wrong?

    // bin_add_byte(section->contents, 0x12);
    // bin_add_byte(section->contents, 0x34);
    // bin_add_byte(section->contents, 0x56);
    // size_t offset = bin_len(section->contents);
    // bin_add_zeros(section->contents, 4);
    // str *sym_name = NULL;
    // section->ops->add_relocation(section, offset, sym_name, 2, -4);
    // section->ops->add_symbol(section, sym_name, offset, ?, true);
}

static obj_module *assemble_listing_into_x86_64_code(assembler *as, asm_listing *asm_list, str *module_name) {
    assembler_data *ad = (assembler_data *)as->priv_data;

    // prepare or clear the module being assembled
    ad->asm_listing = asm_list;
    ad->externs = new_list(ad->mempool); // item is str
    ad->globals = new_list(ad->mempool); // item is str
    ad->module = new_obj_module(ad->mempool);
    ad->module->name = module_name;
    ad->curr_sect = NULL;

    asm_directive *named_def;
    
    // by default we are in code
    str *text = new_str(ad->mempool, ".text");
    ad->curr_sect = ad->module->ops->get_section_by_name(ad->module, text);
    if (ad->curr_sect == NULL) {
        ad->curr_sect = ad->module->ops->create_named_section(ad->module, text);
        ad->module->ops->add_section(ad->module, ad->curr_sect);
    }
    
    for_list(asm_list->lines, asm_line, line) {
        switch (line->type) {
            case ALT_EMPTY:
                break; 

            case ALT_SECTION:  // e.g. ".section .data"
                // find or create section
                named_def = line->per_type.named_definition;
                ad->curr_sect = ad->module->ops->get_section_by_name(ad->module, named_def->name);
                if (ad->curr_sect == NULL) {
                    ad->curr_sect = ad->module->ops->create_named_section(ad->module, named_def->name);
                    ad->module->ops->add_section(ad->module, ad->curr_sect);
                }
                break;

            case ALT_EXTERN:   // e.g. ".extern <name>"
                // add to externs if not already there
                named_def = line->per_type.named_definition;
                if (list_find_first(ad->externs, compare_str, named_def->name) == -1)
                    list_add(ad->externs, named_def->name);
                break;

            case ALT_GLOBAL:   // e.g. ".global <name>"
                // add to globals if not already there
                named_def = line->per_type.named_definition;
                if (list_find_first(ad->globals, compare_str, named_def->name) == -1)
                    list_add(ad->globals, named_def->name);
                break;

            case ALT_DATA:    // "<name:> db, dw, dd, dq value [, value [, ...]]"
                // add to symbols of current section, see if it is extern or glonal
                asm_data_definition *data_def = line->per_type.data_definition;
                obj_symbol *sym = ad->curr_sect->ops->find_symbol(ad->curr_sect, data_def->name, false);
                if (sym != NULL) {
                    error("Symbol '%s' already defined!", str_charptr(data_def->name));
                    return NULL;
                }
                bool is_global = list_find_first(ad->globals, compare_str, data_def->name) != -1;
                bool is_extern = list_find_first(ad->externs, compare_str, data_def->name) != -1;
                size_t value = bin_len(ad->curr_sect->contents);
                bin_cat(ad->curr_sect->contents, data_def->initial_value);
                sym = ad->curr_sect->ops->add_symbol(ad->curr_sect, data_def->name, value, data_def->length_bytes, is_global);
                break;
                
            case ALT_INSTRUCTION:  // MOV RAX, 0x1234
                asm_instruction *instr = line->per_type.instruction;
                encode_instruction_line_x86_64(ad, line);
                if (errors_count) return NULL;
                break;

            default:
                error("Unsupported assembly line type %d", line->type);
                return NULL;
        }
    }

    // now that we assembled all the module, one optimization might be,
    // to go back and resolve local relocations (e.g. if/else end labels)
    // these can be local jumps to offset relative to the end of the instruction,
    // or to function label + offset
    // another way is to have local jumps as addendums from the start of the function

    return ad->module;
}



#ifdef INCLUDE_UNIT_TESTS
static void test_simple_assembly();
static void test_instructions_encoding();
#define verify_instr_encoding(asm, bytes, len)  __verify_instr_encoding(asm, bytes, len, __LINE__)
static void __verify_instr_encoding(asm_line *line, char *expected_bytes, int expected_len, int line_no);

void assembler_unit_tests() {
    // at the least test the encoding of some instructions.
    // maybe test globals and externs,
    // maybe test creation and allocation of data in a data section
    test_simple_assembly();
    test_instructions_encoding();
}

static void test_simple_assembly() {
    mempool *mp = new_mempool();

    str *name = new_str(mp, "file1.c");
    str *func1 = new_str(mp, "func1");
    str *func2 = new_str(mp, "func2");
    asm_listing *list = new_asm_listing(mp);
    list->ops->add_line(list, new_asm_line_directive_global(mp, func1));
    list->ops->set_next_label(list, "%s", str_charptr(func1));
    list->ops->add_line(list, new_asm_line_instruction(mp, OC_RET));
    list->ops->set_next_label(list, "%s", str_charptr(func2));
    list->ops->add_line(list, new_asm_line_instruction(mp, OC_RET));

    assembler *as = new_assembler(mp);

    // ensure we can assemble to a module.
    obj_module *mod = as->ops->assemble_listing_into_x86_64_code(as, list, name);
    assert(mod != NULL);
    assert(str_equals(mod->name, name));

    // ensure sections are in there
    assert(list_length(mod->sections) > 0);
    obj_section *sect = list_get(mod->sections, 0);
    assert(sect != NULL);
    assert(str_cmps(sect->name, ".text") == 0);

    // ensure section symbols are in there
    assert(list_length(sect->symbols) > 0);

    obj_symbol *sym = list_get(sect->symbols, 0);
    assert(sym != NULL);
    assert(sym->global == true); // we marked as global
    assert(str_equals(sym->name, func1));
    assert(sym->value == 0);

    sym = list_get(sect->symbols, 1);
    assert(sym != NULL);
    assert(sym->global == false); // not global
    assert(str_equals(sym->name, func2));
    assert(sym->value == 1);

    // ensure encoded code is in there
    assert(bin_len(sect->contents) > 0);
    assert(memcmp(bin_ptr_at(sect->contents, 0), "\xC3\xC3\x00", 3) == 0);

    mempool_release(mp);
}

void test_instructions_encoding() {
    mempool *mp = new_mempool();
    asm_line *l;

    // this was the easiest!
    verify_instr_encoding(new_asm_line_instruction(mp, OC_RET), "\xC3", 1);

    //       88 d1               mov    cl,dl
    // 66    89 d1               mov    cx,dx
    //       89 d1               mov    ecx,edx
    // 48    89 d1               mov    rcx,rdx
    l = new_asm_line_instruction_reg_reg(mp, OC_MOV, REG_CL, REG_DL);
    verify_instr_encoding(l, "\x88\xd1", 2);
    l = new_asm_line_instruction_reg_reg(mp, OC_MOV, REG_CX, REG_DX);
    verify_instr_encoding(l, "\x66\x89\xd1", 3);
    l = new_asm_line_instruction_reg_reg(mp, OC_MOV, REG_ECX, REG_EDX);
    verify_instr_encoding(l, "\x89\xd1", 2);
    l = new_asm_line_instruction_reg_reg(mp, OC_MOV, REG_RCX, REG_RDX);
    verify_instr_encoding(l, "\x48\x89\xd1", 3);

    //       b1    55            mov    cl,0x55
    // 66    b9    66 55         mov    cx,0x5566
    //       b9    77 66 55 00   mov    ecx,0x556677
    // 48    c7 c1 55 44 33 22   mov    rcx,0x22334455
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_CL, 0x55);
    verify_instr_encoding(l, "\xC6\xC1\x55", 3);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_CX, 0x5566);
    verify_instr_encoding(l, "\x66\xC7\xC1\x66\x55", 5);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_ECX, 0x556677);
    verify_instr_encoding(l, "\xC7\xC1\x77\x66\x55\x00", 6);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_RCX, 0x22334455);
    verify_instr_encoding(l, "\x48\xc7\xc1\x55\x44\x33\x22", 7);

    //       b2    aa            mov    dl,0xaa
    // 66    ba    aa bb         mov    dx,0xbbaa
    //       ba    aa bb cc 00   mov    edx,0xccbbaa
    // 48    c7 c2 aa bb cc 00   mov    rdx,0xccbbaa
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_DL, 0xAA);
    verify_instr_encoding(l, "\xC6\xC2\xAA", 3);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_DX, 0xBBAA);
    verify_instr_encoding(l, "\x66\xC7\xC2\xaa\xbb", 5);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_EDX, 0xCCBBAA);
    verify_instr_encoding(l, "\xC7\xC2\xaa\xbb\xcc\x00", 6);
    l = new_asm_line_instruction_reg_imm(mp, OC_MOV, REG_RDX, 0xCCBBAA);
    verify_instr_encoding(l, "\x48\xc7\xc2\xaa\xbb\xcc\x00", 7);

    // 67    88 0a              mov    BYTE PTR [edx],cl  (size is deduced by source)
    // 67 66 89 0a              mov    WORD PTR [edx],cx
    // 67    89 0a              mov    DWORD PTR [edx],ecx
    // 67 48 89 0a              mov    QWORD PTR [edx],rcx <--- we don't support this.
    l = new_asm_line_instruction_mem_reg(mp, OC_MOV, REG_EDX, REG_CL);
    verify_instr_encoding(l, "\x67\x88\x0a", 3);
    l = new_asm_line_instruction_mem_reg(mp, OC_MOV, REG_EDX, REG_CX);
    verify_instr_encoding(l, "\x67\x66\x89\x0a", 4);
    l = new_asm_line_instruction_mem_reg(mp, OC_MOV, REG_EDX, REG_ECX);
    verify_instr_encoding(l, "\x67\x89\x0a", 3);
    // l = new_asm_line_instruction_mem_reg(mp, OC_MOV, REG_EDX, REG_RDX);
    // verify_instr_encoding(l, "\x67\x48\x89\x0a", 4);

    //       c6 02 aa            mov    BYTE PTR [rdx],0xaa
    // 66    c7 02 aa bb         mov    WORD PTR [rdx],0xbbaa
    //       c7 02 aa bb cc 00   mov    DWORD PTR [rdx],0xccbbaa
    // 48    c7 02 aa bb cc 00   mov    QWORD PTR [rdx],0xccbbaa
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_RDX, 8, 0xAA);
    verify_instr_encoding(l, "\xc6\x02\xaa", 3);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_RDX, 16, 0xBBAA);
    verify_instr_encoding(l, "\x66\xc7\x02\xaa\xbb", 5);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_RDX, 32, 0xCCBBAA);
    verify_instr_encoding(l, "\xc7\x02\xaa\xbb\xcc\x00", 6);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_RDX, 64, 0xCCBBAA);
    verify_instr_encoding(l, "\x48\xc7\x02\xaa\xbb\xcc\x00", 7);

    // 67    c6 02 aa            mov    BYTE PTR [edx],0xaa
    // 67 66 c7 02 aa bb         mov    WORD PTR [edx],0xbbaa
    // 67    c7 02 aa bb cc 00   mov    DWORD PTR [edx],0xccbbaa
    // 67 48 c7 02 aa bb cc 00   mov    QWORD PTR [edx],0xccbbaa
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_EDX, 8, 0xAA);
    verify_instr_encoding(l, "\x67\xc6\x02\xaa", 4);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_EDX, 16, 0xBBAA);
    verify_instr_encoding(l, "\x67\x66\xc7\x02\xaa\xbb", 6);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_EDX, 32, 0xCCBBAA);
    verify_instr_encoding(l, "\x67\xc7\x02\xaa\xbb\xcc\x00", 7);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_EDX, 64, 0xCCBBAA);
    verify_instr_encoding(l, "\x67\x48\xc7\x02\xaa\xbb\xcc\x00", 8);

    // 67    c7 00 66 55 00 00   mov    DWORD PTR [eax],0x5566  (67 to set the pointer to Exx instead of Rxx, AX/AL are not used)
    //       c7 00 66 55 00 00   mov    DWORD PTR [rax],0x5566
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_EAX, 32, 0x5566);
    verify_instr_encoding(l, "\x67\xc7\x00\x66\x55\x00\x00", 7);
    l = new_asm_line_instruction_mem_imm(mp, OC_MOV, REG_RAX, 32, 0x5566);
    verify_instr_encoding(l, "\xc7\x00\x66\x55\x00\x00", 6);

    mempool_release(mp);
}

static void __verify_instr_encoding(asm_line *line, char *expected_bytes, int expected_len, int line_no) {
    mempool *mp = new_mempool();
    str *name = new_str(mp, "unit_test.c");
    asm_listing *l = new_asm_listing(mp);
    l->ops->add_line(l, line);

    assembler *as = new_assembler(mp);
    obj_module *m = as->ops->assemble_listing_into_x86_64_code(as, l, name);
    assertm(m != NULL, "Failed assembling instruction, no module was returned!");
    if (m == NULL) return;
    obj_section *s = list_get(m->sections, 0);
    bin *expected = new_bin_from_mem(mp, expected_bytes, expected_len);

    bool match = (bin_cmp(s->contents, expected) == 0);
    str *instr_descrip = str_trim(asm_line_to_str(mp, line), new_str(mp, " "));
    str *exp_hex = bin_to_hex_str(expected, mp);
    str *got_hex = bin_to_hex_str(s->contents, mp);
    str *msg = new_strf(mp, "Bad instruction encoding at line %d:\n  '%s', expected %s, got %s", 
        line_no, str_charptr(instr_descrip), str_charptr(exp_hex), str_charptr(got_hex));
    assertm(match, str_charptr(msg));

    mempool_release(mp);
}
#endif
