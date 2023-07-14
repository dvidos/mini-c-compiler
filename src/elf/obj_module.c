#include <string.h>
#include "obj_module.h"
#include "elf_format.h"

static void obj_module_print(obj_module *module, bool show_details, FILE *file);
static void obj_module_append(obj_module *module, obj_module *source);
static obj_section *obj_module_get_section_by_name(obj_module *m, str *name);
static obj_section *obj_module_add_section(obj_module *m, str *name);
static obj_symbol *obj_module_find_symbol(obj_module *module, str *name, bool exported);
static elf64_contents *obj_module_prepare_elf_contents(obj_module *module, int elf_type, mempool *mp);

static struct obj_module_ops module_ops = {
    .print = obj_module_print,
    .get_section_by_name = obj_module_get_section_by_name,
    .add_section = obj_module_add_section,
    .find_symbol = obj_module_find_symbol,
    .prepare_elf_contents = obj_module_prepare_elf_contents,
};


obj_module *new_obj_module(mempool *mp) {
    obj_module *m = mpalloc(mp, obj_module);
    m->name = new_str(mp, "");
    m->sections = new_list(mp);

    m->ops = &module_ops;
    m->mempool = mp;
    return m;
}


#define PACKING_MAX_SECTIONS  16

// use this packing_info struct to keep track between obj_modules and elf_modules
struct packing_info {
    obj_module *module;
    elf64_contents *elf;
    struct section_info {
        obj_section *obj;
        elf64_section *elf;
        elf64_section *elf_rela;
    } sections[PACKING_MAX_SECTIONS];
    int sections_len;
    elf64_section *symtab;
    elf64_section *strtab;
    elf64_section *shstrtab;
};


static elf64_contents *obj_module_prepare_elf_contents(obj_module *module, int elf_type, mempool *mp) {
    // use this packing_info struct to keep track between obj_modules and elf_modules
    struct packing_info packing_info;
    struct packing_info *pi = &packing_info;



    // prepare the runtime packing info, to allow us to sync between obj and elf sections.
    memset(pi, 0, sizeof(struct packing_info));
    pi->module = module;
    pi->elf = new_elf64_contents(mp);
    pi->symtab = pi->elf->ops->create_section(pi->elf, new_str(mp, ".symtab"), SECTION_TYPE_SYMTAB);
    pi->strtab = pi->elf->ops->create_section(pi->elf, new_str(mp, ".strtab"), SECTION_TYPE_STRTAB);
    pi->shstrtab = pi->elf->ops->create_section(pi->elf, new_str(mp, ".shstrtab"), SECTION_TYPE_STRTAB);

    // mark this as a relocatable or executable file
    pi->elf->header->file_type = elf_type;

    // add the empty symbol to the symbol table and the FILE name
    pi->symtab->ops->add_named_symbol(pi->symtab, new_str(mp, ""), 0, 0, STT_NOTYPE, 0, 0, pi->strtab);
    pi->symtab->ops->add_named_symbol(pi->symtab, module->name, 0, 0, STT_FILE, STB_LOCAL, SHN_ABS, pi->strtab);
    
    // add the empty section first (index 0)
    pi->elf->ops->add_section(pi->elf, 
        pi->elf->ops->create_section(pi->elf, new_str(mp, ""), SECTION_TYPE_NULL));
    
    // add all sections to elf to get an index
    for_list(module->sections, obj_section, obj_sect) {
        elf64_section *elf_sect = pi->elf->ops->create_section(pi->elf, obj_sect->name, 
            obj_sect->flags.init_to_zero ? SECTION_TYPE_NOBITS : SECTION_TYPE_PROGBITS);
        elf_sect->header->flags = 
            (obj_sect->flags.allocate ? SECTION_FLAGS_ALLOC : 0) |
            (obj_sect->flags.writable ? SECTION_FLAGS_WRITE : 0) |
            (obj_sect->flags.executable ? SECTION_FLAGS_EXECINSTR : 0);
        elf_sect->header->virt_address = obj_sect->address; // needed for program_headers
        elf_sect->header->address_alignment = obj_sect->flags.executable ? 1 : 4;
        bin_cpy(elf_sect->contents, obj_sect->contents);
        pi->elf->ops->add_section(pi->elf, elf_sect);

        elf64_section *rela_sect = NULL;
        if (!list_is_empty(obj_sect->relocations)) {
            str *rela_name = new_str(pi->elf->mempool, ".rela");
            str_cat(rela_name, obj_sect->name);
            rela_sect = pi->elf->ops->create_section(pi->elf, rela_name, SECTION_TYPE_RELA);
            pi->elf->ops->add_section(pi->elf, rela_sect);
        }

        // note this on pack_info, for later backfilling
        pi->sections[pi->sections_len].obj = obj_sect;
        pi->sections[pi->sections_len].elf = elf_sect;
        pi->sections[pi->sections_len].elf_rela = rela_sect;
        pi->sections_len += 1;
    }

    // after all PROGBITS / NOBITS sections, add the symbol and strings tables
    pi->elf->ops->add_section(pi->elf, pi->symtab);
    pi->elf->ops->add_section(pi->elf, pi->strtab);
    pi->elf->ops->add_section(pi->elf, pi->shstrtab);

    // now that we have all sections added, populate the header names table
    for_list(pi->elf->sections, elf64_section, sect) {
        sect->header->name = pi->shstrtab->ops->add_strz_get_offset(pi->shstrtab, sect->name);
    }

    // now add all symbols to symtab, since we have indexes on the relevant sections
    // first only local symbols
    for (int i = 0; i < pi->sections_len; i++) {
        for_list(pi->sections[i].obj->symbols, obj_symbol, sym) {
            if (sym->global) continue;
            pi->symtab->ops->add_named_symbol(pi->symtab,
                sym->name, sym->value, sym->size, 
                pi->sections[i].obj->flags.executable ? STT_FUNC : STT_OBJECT,
                sym->global ? STB_GLOBAL : STB_LOCAL, 
                pi->sections[i].elf->index, pi->strtab);
        }
    }

    // save the index of the first global symbol in the symbol table
    size_t first_global_symbol_index = pi->symtab->ops->count_symbols(pi->symtab);

    // now only global symbols
    for (int i = 0; i < pi->sections_len; i++) {
        for_list(pi->sections[i].obj->symbols, obj_symbol, sym) {
            if (!sym->global) continue;
            pi->symtab->ops->add_named_symbol(pi->symtab,
                sym->name, sym->value, sym->size, 
                pi->sections[i].obj->flags.executable ? STT_FUNC : STT_OBJECT,
                sym->global ? STB_GLOBAL : STB_LOCAL, 
                pi->sections[i].elf->index, pi->strtab);
        }
    }

    // now insert all the relocations, allow them to create UNDefined symbols
    for (int i = 0; i < pi->sections_len; i++) {
        if (pi->sections[i].elf_rela == NULL) continue;
        for_list(pi->sections[i].obj->relocations, obj_relocation, rel) {
            pi->sections[i].elf_rela->ops->add_named_relocation(pi->sections[i].elf_rela,
                rel->offset, rel->symbol_name, rel->type, rel->addendum, 
                pi->symtab, pi->strtab);
        }
    }
    // update sections links and infos:
    // - for relas, link=sym, info=target
    // - for syms,  link=str, info=first-global
    for (int i = 0; i < pi->sections_len; i++) {
        if (pi->sections[i].elf_rela != NULL) {
            pi->sections[i].elf_rela->header->link = pi->symtab->index;
            pi->sections[i].elf_rela->header->info = pi->sections[i].elf->index;
        }
    }

    pi->symtab->header->entry_size = sizeof(elf64_sym);
    pi->symtab->header->info = first_global_symbol_index;
    pi->symtab->header->link = pi->strtab->index;

    if (elf_type == ELF_TYPE_EXEC) {
        obj_symbol *start = obj_module_find_symbol(pi->module, new_str(mp, "_start"), true);
        if (start == NULL) {
            printf("Symbol _start not found\n");
            return NULL;
        }
        pi->elf->header->entry_point = start->value;
    }

    // I think we are done!
    return pi->elf;
}

// ---------------------------------------------------------------------------

static obj_symbol *new_obj_symbol_from_elf_symbol(elf64_sym *sym, elf64_section *strtab, list *sections, mempool *mp) {
    obj_symbol *s = mpalloc(mp, obj_symbol);
    s->name = new_str(mp, "");

    int type = ELF64_ST_TYPE(sym->st_info);
    if (type == STT_SECTION) {  // section names are not contained in the string table
        elf64_section *section = list_get(sections, sym->st_shndx);
        if (section != NULL)
            str_cat(s->name, section->name);
    } else {
        str_cats(s->name, bin_ptr_at(strtab->contents, sym->st_name));
    }

    s->global = ELF64_ST_BIND(sym->st_info) == STB_GLOBAL;
    s->value = sym->st_value;
    s->size = sym->st_size;

    return s;
}

static obj_relocation *new_obj_relocation_from_elf_relocation(elf64_rela *rel, elf64_section *symtab, elf64_section *strtab, list *elf64_sections, mempool *mp) {
    obj_relocation *r = mpalloc(mp, obj_relocation);

    // find the symbol this relocation needs to resolve
    size_t sym_index = ELF64_R_SYM(rel->r_info);
    elf64_sym *sym = bin_ptr_at(symtab->contents, sym_index * sizeof(elf64_sym));
    r->symbol_name = new_str(mp, "");

    int sym_type = ELF64_ST_TYPE(sym->st_info);
    if (sym_type == STT_SECTION) {  // section names are not contained in the string table
        elf64_section *section = list_get(elf64_sections, sym->st_shndx);
        if (section != NULL)
            str_cat(r->symbol_name, section->name);
    } else {
        str_cats(r->symbol_name, bin_ptr_at(strtab->contents, sym->st_name));
    }

    r->offset = rel->r_offset;
    r->addendum = rel->r_addend;
    r->type = ELF64_R_TYPE(rel->r_info);

    return r;
}

static bool is_unsupported_elf64_section(elf64_section *s) {
    // encountering these means we cannot safely load the object file
    return 
        (s->header->type == SECTION_TYPE_HASH) ||
        (s->header->type == SECTION_TYPE_DYNAMIC) ||
        (s->header->type == SECTION_TYPE_REL) ||
        (s->header->type == SECTION_TYPE_SHLIB) ||
        (s->header->type == SECTION_TYPE_DYNSYM) ||
        (s->header->type == SECTION_TYPE_NUM);
}

static bool should_ignore_elf64_section(elf64_section *s) {
    if (s->header->type == SECTION_TYPE_NULL) {
        return true; // the first section can always be ignored
    } else if (s->header->type == SECTION_TYPE_NOTE) {
        if (str_cmps(s->name, ".note.gnu.property") == 0) {
            // has something to do with special flags the GNU compiler passes,
            // such as GNU_PROPERTY_X86_FEATURE_1_IBT
            return true;
        }
    } else if (s->header->type == SECTION_TYPE_PROGBITS) {
        if (str_cmps(s->name, ".comment") == 0) {
            return true; // gcc compiler / system version
        } else if (str_cmps(s->name, ".note.GNU-stack") == 0) {
            // about executable stack, not 100% clear
            // https://wiki.gentoo.org/wiki/Hardened/GNU_stack_quickstart
            return true;
        }
    } else if (s->header->type == SECTION_TYPE_STRTAB) {
        if (str_cmps(s->name, ".shstrtab") == 0) {
            return true; // section names should already be parsed
        }
    }

    return false;
}

obj_section *create_obj_section_from_elf_section(elf64_section *elf_sect, mempool *mp) {
    obj_section *obj_sect = new_obj_section(mp);

    obj_sect->name = elf_sect->name;
    obj_sect->flags.writable     = (elf_sect->header->flags & SECTION_FLAGS_WRITE) != 0;
    obj_sect->flags.allocate     = (elf_sect->header->flags & SECTION_FLAGS_ALLOC) != 0;
    obj_sect->flags.executable   = (elf_sect->header->flags & SECTION_FLAGS_EXECINSTR) != 0;
    obj_sect->flags.init_to_zero = (elf_sect->header->type == SECTION_TYPE_NOBITS);
    bin_cpy(obj_sect->contents, elf_sect->contents);

    return obj_sect;
}

void parse_relocations_section(elf64_contents *contents, elf64_section *rela_section, obj_section *target_section, elf64_section *symtab, elf64_section *strtab, mempool *mp) {
    int num_relocations = bin_len(rela_section->contents) / sizeof(elf64_rela);
    elf64_rela *rela;

    for (int i = 0; i < num_relocations; i++) {
        rela = (elf64_rela *)bin_ptr_at(rela_section->contents, i * sizeof(elf64_rela));
        obj_relocation *r = new_obj_relocation_from_elf_relocation(rela, symtab, strtab, contents->sections, mp);
        list_add(target_section->relocations, r);
    }
}

bool parse_symbols_table(elf64_section *symtab, elf64_section *strtab, obj_section **obj_sections_per_elf_index, obj_module *module, list *elf_sections, mempool *mp) {
    int num_symbols = bin_len(symtab->contents) / sizeof(elf64_sym);
    elf64_sym *elf_sym;

    for (int i = 0; i < num_symbols; i++) {
        elf_sym = (elf64_sym *)bin_ptr_at(symtab->contents, i * sizeof(elf64_sym));
        if (elf_sym->st_shndx == 0)
            continue;

        obj_symbol *obj_sym = new_obj_symbol_from_elf_symbol(elf_sym, strtab, elf_sections, mp);
        if (ELF64_ST_TYPE(elf_sym->st_info) == STT_FILE) {
            module->name = obj_sym->name;
            continue;
        }

        obj_section *target_section = obj_sections_per_elf_index[elf_sym->st_shndx];
        if (target_section == NULL) {
            // "Uknown symbol owner (name 'msg', section '.data.rel.local', type 1)"
            printf("Symbol %s refers to elf section %d, which is not parsed\n", str_charptr(obj_sym->name), elf_sym->st_shndx);
            return false;
        }
        list_add(target_section->symbols, obj_sym);
    }
    return true;
}

obj_module *new_obj_module_from_elf64_contents(elf64_contents *contents, mempool *mp) {
    obj_module *module = new_obj_module(mp);

    // to temporarily store the elf_section <--> obj_section relationship
    obj_section *obj_sections_per_elf_index[32];
    memset(obj_sections_per_elf_index, 0, sizeof(obj_sections_per_elf_index));

    // let's go over all the sections and see how we can tackle each one
    // printf("Let's see how to import elf64 contents into our obj_sections....\n");
    iterator *sections_it = list_create_iterator(contents->sections, mp);
    for_iterator(elf64_section, s, sections_it) {
        // printf("- %2d: %s, type %d\n", s->index, str_charptr(s->name), s->header->type);
        if (is_unsupported_elf64_section(s)) {
            printf("Unsupported section '%s' of type %d, aborting", str_charptr(s->name), s->header->type);
            return NULL;
        } else if (should_ignore_elf64_section(s)) {
            continue;
        }

        if (s->header->type == SECTION_TYPE_PROGBITS || s->header->type == SECTION_TYPE_NOBITS) {
            obj_section *os = create_obj_section_from_elf_section(s, mp);
            list_add(module->sections, os);
            obj_sections_per_elf_index[s->index] = os;

        } else if (s->header->type == SECTION_TYPE_RELA) {
            // we must enrich the appropriate section
            int target_section_index = s->header->info;
            int relevant_symbol_table = s->header->link;
            obj_section *target_obj_section = obj_sections_per_elf_index[target_section_index];
            if (target_obj_section == NULL) {
                printf("Relocation section %s points to index %d, not parsed yet\n", str_charptr(s->name), target_section_index);
                return NULL;
            }
            elf64_section *symbols_section = list_get(contents->sections, relevant_symbol_table);
            elf64_section *strtab_section = list_get(contents->sections, symbols_section->header->link);
            parse_relocations_section(contents, s, target_obj_section, symbols_section, strtab_section, mp);
            
        } else if (s->header->type == SECTION_TYPE_SYMTAB) {
            int relevant_string_table = s->header->link;
            int index_of_first_global_symbol = s->header->info;
            elf64_section *strtab_section = list_get(contents->sections, s->header->link);
            // parse the symbols, find relevant sections, and distribute them.
            if (!parse_symbols_table(s, strtab_section, obj_sections_per_elf_index, module, contents->sections, mp))
                return NULL;

        } else if (s->header->type == SECTION_TYPE_STRTAB) {
            ; // we don't do anything, symbols should have been parsed already
        } else {
            printf("  Section '%s' is of unknwon type %d, not supported\n", str_charptr(s->name), s->header->type);
            return NULL;
        }
    }

    return module;
}

static void obj_module_print(obj_module *module, bool show_details, FILE *file) {
    // print each section with it's symbols and relocations
    fprintf(file, "Module '%s'\n", str_charptr(module->name));

    for_list(module->sections, obj_section, s) {
        if (bin_len(s->contents) == 0) continue;
        s->ops->print(s, show_details, file);
    }
}

static int compare_section_and_name(obj_section *s, str *name) {
    return str_cmp(s->name, name);
}

static obj_section *obj_module_get_section_by_name(obj_module *m, str *name) {
    int index = list_find_first(m->sections, (comparator_func*)compare_section_and_name, name);
    return index == -1 ? NULL : list_get(m->sections, index);
}

static obj_section *obj_module_add_section(obj_module *m, str *name) {
    obj_section *section = new_obj_section(m->mempool);
    section->name = name;
    list_add(m->sections, section);
    return section;
}

static obj_symbol *obj_module_find_symbol(obj_module *m, str *name, bool exported) {
    obj_symbol *sym;

    for_list (m->sections, obj_section, s) {
        sym = s->ops->find_symbol(s, name, exported);
        if (sym != NULL) return sym;
    }
    
    return NULL;
}



