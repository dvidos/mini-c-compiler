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

static void encode_memreg_reg_instruction(assembler_data *ad, u8 op_code, 
    asm_reg_or_mem_operand *memreg_op, gp_register reg) {
    // base opcde + ModRegRM
}

static void encode_memreg_imm_instruction(assembler_data *ad, u8 op_code, u8 op_extension,
    asm_reg_or_mem_operand *regmem, long immediate) {
    // base opcode, ModRm to set destination, immediate follows
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
            // what is different between the rows?
            // let's skip optimizing for one byte, for now.
            if (instr->regimm_operand.is_register) {
                /*  88/r mov rm8 <- r8
                    89/r mov rm16/32/64 <- r16/32/64
                    8A/r mov rm8 -> r8
                    8B/r mov rm16/32/64 -> r16/32/64 
                */
                encode_memreg_reg_instruction(ad,
                    instr->direction_rm_to_ri_operands ? 0x8B : 0x89,
                    &instr->regmem_operand, instr->regimm_operand.per_type.reg);
            } else if (instr->regimm_operand.is_immediate) {
                /*  B0+r mov r8 <- imm8
                    B8+r mov r16/32/64 <- imm16/32/64
                    C6/0 mov rm8 <- imm8
                    C7/0 mov rm16/32/64 <- imm16/32/64
                */
                encode_memreg_imm_instruction(ad, 0xC7, 0,
                    &instr->regmem_operand, instr->regimm_operand.per_type.immediate);
            }
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

void assembler_unit_tests() {
    // at the least test the encoding of some instructions.
    // maybe test globals and externs,
    // maybe test creation and allocation of data in a data section
    test_simple_assembly();
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
#endif
