#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../err_handler.h"
#include "../run_info.h"
#include "../utils/string.h"
#include "../compiler/codegen/ir_listing.h"
#include "../linker/obj_code.h"
#include "encoder/encoding_info.h"
#include "encoder/encoded_instruction.h"
#include "encoder/encoder.h"
#include "encoder/asm_allocator.h"
#include "asm_line.h"
#include "asm_listing.h"
#include "../utils/string.h"
#include "../utils/data_structs.h"
#include "../elf/obj_module.h"
#include "../utils/data_types.h"

void assemble_listing_into_i386_code(mempool *mp, asm_listing *asm_list, obj_code *obj) {

    // encode this into intel machine code
    struct x86_encoder *enc = new_x86_encoder(mp, obj->text->contents, obj->text->relocations);
    asm_instruction *inst;
    struct encoding_info enc_info;
    encoded_instruction enc_inst;

    for_list(asm_list->lines, asm_line, line) {
        inst = line->per_type.instruction;

        if (line->label != NULL) {
            // we don't know if this is exported for now
            obj->text->symbols->add(obj->text->symbols, str_charptr(line->label), 
                obj->text->contents->length, 0, ST_FUNCTION, false);
        }

        if (inst->operation == OC_NONE)
            continue;
        
        if (!enc->encode_v4(enc, inst)) {
            str *s = new_str(mp, NULL);
            asm_instruction_to_str(inst, s, false);
            error(NULL, 0, "Failed encoding instruction: '%s'\n", str_charptr(s));
            continue;
        }
    }

    printf("Sample resulting machine code\n");
    for (int i = 0; i < obj->text->contents->length; i++)
        printf("%02x ", (unsigned char)obj->text->contents->buffer[i]);
    printf("\n");
}


// ------------------------------------------------

static void encode_instruction_x86_64(asm_instruction *instr, obj_section *section) {
    // ...
}

static int compare_str(const void *a, const void *b) {
    return str_cmp((str *)a, (str *)b);
}

void assemble_listing_into_x86_64_code(mempool *mp, asm_listing *asm_list, obj_module *target) {

    llist *externs = new_llist(mp); // item is str
    llist *globals = new_llist(mp); // item is str
    asm_named_definition *named_def;
    
    // by default we are in code
    str *text_name = new_str(mp, ".text");
    obj_section *curr_sect = target->ops->get_section_by_name(target, text_name);
    if (curr_sect == NULL)
        curr_sect = target->ops->add_section(target, text_name);
    
    for_list(asm_list->lines, asm_line, line) {
        switch (line->type) {
            case ALT_EMPTY:
                break; 

            case ALT_SECTION:  // e.g. ".section .data"
                // find or create section
                named_def = line->per_type.named_definition;
                curr_sect = target->ops->get_section_by_name(target, named_def->name);
                if (curr_sect == NULL)
                    curr_sect = target->ops->add_section(target, named_def->name);
                break;

            case ALT_EXTERN:   // e.g. ".extern <name>"
                // add to externs if not already there
                named_def = line->per_type.named_definition;
                if (llist_find_first(externs, compare_str, named_def->name) == -1)
                    llist_add(externs, named_def->name);
                break;

            case ALT_GLOBAL:   // e.g. ".global <name>"
                // add to globals if not already there
                named_def = line->per_type.named_definition;
                if (llist_find_first(globals, compare_str, named_def->name) == -1)
                    llist_add(globals, named_def->name);
                break;

            case ALT_DATA:    // "<name:> db, dw, dd, dq value [, value [, ...]]"
                // add to symbols of current section, see if it is extern or glonal
                asm_data_definition *data_def = line->per_type.data_definition;
                obj_symbol *sym = curr_sect->ops->find_symbol(curr_sect, data_def->name, false);
                if (sym != NULL) {
                    error(NULL, 0, "Symbol '%s' already defined!", str_charptr(data_def->name));
                    return;
                }
                bool is_global = llist_find_first(globals, compare_str, data_def->name) != -1;
                bool is_extern = llist_find_first(externs, compare_str, data_def->name) != -1;
                size_t value = bin_len(curr_sect->contents);
                bin_cat(curr_sect->contents, data_def->initial_value);
                sym = curr_sect->ops->add_symbol(curr_sect, data_def->name, value, data_def->length_bytes, is_global);
                break;
                
            case ALT_INSTRUCTION:  // MOV RAX, 0x1234
                asm_instruction *instr = line->per_type.instruction;
                encode_instruction_x86_64(instr, curr_sect);
                if (errors_count) return;
                break;

            default:
                error(NULL, 0, "Unsupported assembly line type %d", line->type);
                return;
        }
    }
}

