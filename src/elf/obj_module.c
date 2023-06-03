#include <string.h>
#include "obj_module.h"
#include "elf_format.h"

static obj_section *new_obj_section(mempool *mp, const char *name) {
    obj_section *s = mempool_alloc(mp, sizeof(obj_section), "obj_section");
    s->name = new_str(mp, name);
    s->contents = new_bin(mp);
    s->relocations = new_llist(mp);
    s->symbols = new_llist(mp);
    return s;
}

obj_module *new_obj_module(mempool *mp, const char *name) {
    obj_module *module = mempool_alloc(mp, sizeof(obj_module), "obj_module");
    module->name = new_str(mp, name);
    module->text = new_obj_section(mp, ".text");
    module->data = new_obj_section(mp, ".data");
    module->bss = new_obj_section(mp, ".bss");
    module->rodata = new_obj_section(mp, ".rodata");
    return module;
}

static size_t get_strtab_index(str *s, elf64_section *strtab) {
    // the first byte of the strtab is a terminator to allow for empty strings
    if (str_len(s) == 0) {
        if (bin_len(strtab->contents) == 0)
            bin_add_byte(strtab->contents, 0x00);
        return 0;
    }

    // in case the name already exists, reuse it
    int index = bin_index_of(strtab->contents, str_charptr(s), str_len(s) + 1);
    if (index >= 0)
        return index;

    // otherwise add it, saving address first
    index = bin_len(strtab->contents);
    bin_add_str(strtab->contents, s);

    return index;
}

static void add_elf64_symbol(str *name, size_t value, size_t size, int st_type, bool global, int st_shndx, elf64_section *symtab, elf64_section *strtab) {
    elf64_sym sym;
    sym.st_name = get_strtab_index(name, strtab);
    sym.st_value = value;
    sym.st_size = size;
    sym.st_info = ELF64_ST_INFO(global ? STB_GLOBAL : STB_LOCAL, st_type);
    sym.st_other = 0;
    sym.st_shndx = st_shndx;
    bin_add_mem(symtab->contents, &sym, sizeof(elf64_sym));
}

static void pack_elf64_symbols(llist *symbols_list, int st_type, int st_shndx, mempool *mp, elf64_section *symtab, elf64_section *strtab) {
    iterator *it = llist_create_iterator(symbols_list, mp);
    for_iterator(obj_symbol, s, it)
        add_elf64_symbol(s->name, s->value, s->size, st_type, s->global, st_shndx, symtab, strtab);
}

static int find_elf64_symbol(const char *name, elf64_section *symtab, elf64_section *strtab) {
    elf64_sym *sym_ptr;
    char *sym_name;
    int total_syms = bin_len(symtab->contents) / sizeof(elf64_sym);
    for (int i = 0; i < total_syms; i++) {
        sym_ptr = bin_ptr_at(symtab->contents, i * sizeof(elf64_sym));
        char *sym_name = bin_ptr_at(strtab->contents, sym_ptr->st_name);
        if (strcmp(name, sym_name) == 0)
            return i;
    }
    return -1;
};

static int elf64_symbols_table_len(elf64_section *symtab) {
    return bin_len(symtab->contents) / sizeof(elf64_sym);
};

static void pack_elf64_relocations(llist *relocations_list, mempool *mp, elf64_section *relatab, elf64_section *symtab, elf64_section *strtab) {
    elf64_rela rela;

    iterator *it = llist_create_iterator(relocations_list, mp);
    for_iterator(obj_relocation, r, it) {
        // we need to find the symbol number, look it up?
        // if it does not exist, we should include it with the UNDEFINED index, NOTYPE type, GLOBAL visitibilty
        int symbol_num = find_elf64_symbol(str_charptr(r->symbol_name), symtab, strtab);
        if (symbol_num == -1) {
            symbol_num = elf64_symbols_table_len(symtab);
            add_elf64_symbol(r->symbol_name, 0, 0, STT_NOTYPE, true, SHN_UNDEF, symtab, strtab);
        }

        rela.r_offset = r->offset;
        rela.r_info = ELF64_R_INFO(symbol_num, r->type);
        rela.r_addend = r->addendum;

        // add this relocation
        bin_add_mem(relatab->contents, &rela, sizeof(elf64_rela));
    }
}

static elf64_section *add_elf64_section(elf64_contents *contents, str *name, bin *section_contents, int type, mempool *mp) {
    elf64_section *section = new_elf64_section(mp);

    section->index = llist_length(contents->sections);
    section->name = name;
    section->header->type = type;

    if (section_contents != NULL)
        bin_cpy(section->contents, section_contents);

    llist_add(contents->sections, section);
    return section;
}

elf64_contents *pack_elf64_object_file(obj_module *module, mempool *mp) {
    elf64_contents *contents = new_elf64_contents(mp);
    iterator *it;

    // mark this as a relocatable file
    contents->header->file_type = ELF_TYPE_REL;

    // the first is the empty section, always (index 0)
    add_elf64_section(contents, new_str(mp, ""), NULL, SECTION_TYPE_NULL, mp);

    // the next four are fixed sequence (1-4)
    add_elf64_section(contents, new_str(mp, ".text"), module->text->contents, SECTION_TYPE_PROGBITS, mp);
    add_elf64_section(contents, new_str(mp, ".data"), module->data->contents, SECTION_TYPE_PROGBITS, mp);
    add_elf64_section(contents, new_str(mp, ".bss"), module->bss->contents, SECTION_TYPE_NOBITS, mp);
    add_elf64_section(contents, new_str(mp, ".rodata"), module->rodata->contents, SECTION_TYPE_PROGBITS, mp);

    // prepare three sections to populate
    elf64_section *rela_text = add_elf64_section(contents, new_str(mp, ".rela.text"), NULL, SECTION_TYPE_RELA, mp);
    elf64_section *symtab = add_elf64_section(contents, new_str(mp, ".symtab"), NULL, SECTION_TYPE_SYMTAB, mp);
    elf64_section *strtab = add_elf64_section(contents, new_str(mp, ".strtab"), NULL, SECTION_TYPE_STRTAB, mp);
    mempool *scratch = new_mempool();

    // add the empty symbol to the symbol table
    add_elf64_symbol(new_str(mp, ""), 0, 0, STT_NOTYPE, false, SHN_UNDEF, symtab, strtab);

    // add symbol for the file name
    add_elf64_symbol(module->name, 0, 0, STT_FILE, false, SHN_ABS, symtab, strtab);

    // add symbol for each section
    add_elf64_symbol(module->text->name, 0, 0, STT_SECTION, false, 1, symtab, strtab);
    add_elf64_symbol(module->data->name, 0, 0, STT_SECTION, false, 2, symtab, strtab);
    add_elf64_symbol(module->bss->name, 0, 0, STT_SECTION, false, 3, symtab, strtab);
    add_elf64_symbol(module->rodata->name, 0, 0, STT_SECTION, false, 4, symtab, strtab);

    // pack various symbols into songle symtab section (and names into strtab)
    pack_elf64_symbols(module->text->symbols,   STT_FUNC,   1,   scratch, symtab, strtab);
    pack_elf64_symbols(module->data->symbols,   STT_OBJECT, 2,   scratch, symtab, strtab);
    pack_elf64_symbols(module->bss->symbols,    STT_OBJECT, 3,    scratch, symtab, strtab);
    pack_elf64_symbols(module->rodata->symbols, STT_OBJECT, 4, scratch, symtab, strtab);

    // pack relocations as well
    pack_elf64_relocations(module->text->relocations, scratch, rela_text, symtab, strtab);

    mempool_release(scratch);
    return contents;
}


elf64_contents *pack_elf64_executable_file(obj_module *module, mempool *mp) {
    // the difference is that we need to make program headers as well,
    // also, make all the sections align into page boundaries, the headers portion as well.

    /*
    // all loadable sections MUST be page aligned (from linker), otherwise they cannot load
    if (contents->text->header->virt_address & (ALIGN_SIZE - 1)
        || contents->data->header->virt_address & (ALIGN_SIZE - 1)
        || contents->bss->header->virt_address & (ALIGN_SIZE - 1)
        || contents->rodata->header->virt_address & (ALIGN_SIZE - 1))
        return false;
    
    if ((bin_len(contents->text->contents) & (ALIGN_SIZE - 1))
        || (bin_len(contents->data->contents) & (ALIGN_SIZE - 1))
        || (bin_len(contents->bss->contents) & (ALIGN_SIZE - 1))
        || (bin_len(contents->rodata->contents) & (ALIGN_SIZE - 1)))
        return false;

    bin *result = new_bin(mp);
    bin *section_names_table = new_bin(mp);
    
    u64 program_headers_offset = 0;
    u64 program_headers_count = 0;
    u64 section_headers_offset = 0;
    u64 section_headers_count = 0;
    u64 section_names_table_index = 0;
    u64 file_loading_info_length = 0;

    // file header placeholder
    bin_add_zeros(result, sizeof(elf64_header)); 

    if (executable) {
        // we shall create 5 loading headers: headers, text, data, bss, rodata
        program_headers_offset = bin_len(result);
        program_headers_count = 5;

        bin_add_zeros(result, sizeof(elf64_prog_header) * program_headers_count);
        
        // since we are loading the file headers table, pad this to a page size
        bin_pad(result, 0, round_up(bin_len(result), ALIGN_SIZE));
        file_loading_info_length = bin_len(result);
    }

    // do two things for each section:
    // - prepare header, noting current file offset, append name in names_table
    // - add content, to advance file offset

    // Sect#    Name          Notes
    //     0    <null>
    //     1    .text
    //     2    .data
    //     3    .bss
    //     4    .rodata
    //     5    .rela_text    link to 6 for str table, info to 1 for relevant code 
    //     6    .symtab       symbols, link to 7 for strings, info must be "One greater than the symbol table index of the last local symbol (binding STB_LOCAL)"
    //     7    .strtab       names of symbols
    //     8    .comment      
    //     9    .shstrtab     names of sections

    // section 0
    elf64_section_header *empty_section_header = new_elf64_section_header(
        0, "", 0, 
        bin_len(result), 0, 0, 0, 0, 0,
        section_names_table, mp);
    // no contents, this is an empty section
    
    // section 1
    elf64_section_header *text_header = new_elf64_section_header(
        SECTION_TYPE_PROGBITS, ".text", SECTION_FLAGS_ALLOC | SECTION_FLAGS_EXECINSTR, 
        bin_len(result), bin_len(contents->text->contents), 0, 0, 0, 0,
        section_names_table, mp);
    bin_cat(result, contents->text->contents);

    elf64_section_header *data_header = new_elf64_section_header(
        SECTION_TYPE_PROGBITS, ".data", SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE, 
        bin_len(result), bin_len(contents->data->contents), 0, 0, 0, 0,
        section_names_table, mp);
    bin_cat(result, contents->data->contents);

    elf64_section_header *bss_header = new_elf64_section_header(
        SECTION_TYPE_NOBITS, ".bss", SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE, 
        bin_len(result), bin_len(contents->bss->contents), 0, 0, 0, 0,
        section_names_table, mp);
    // bin_cat(file_buffer, contents->rodata);  no storage for BSS segment

    elf64_section_header *rodata_header = new_elf64_section_header(
        SECTION_TYPE_PROGBITS, ".rodata", SECTION_FLAGS_ALLOC, 
        bin_len(result), bin_len(contents->rodata->contents), 0, 0, 0, 0, 
        section_names_table, mp);
    bin_cat(result, contents->rodata->contents);

    elf64_section_header *rela_text_header = new_elf64_section_header(
        SECTION_TYPE_RELA, ".rela.text", 0, 
        bin_len(result), bin_len(contents->rela_text->contents), sizeof(elf64_rela), 6, 1, 0,  
        section_names_table, mp);
    bin_cat(result, contents->rela_text->contents);

    elf64_section_header *symtab_header = new_elf64_section_header(
        SECTION_TYPE_SYMTAB, ".symtab", 0, 
        bin_len(result), bin_len(contents->symtab->contents), sizeof(elf64_sym), 7, 1, 0, 
        section_names_table, mp);
    bin_cat(result, contents->symtab->contents);

    elf64_section_header *strtab_header = new_elf64_section_header(
        SECTION_TYPE_STRTAB, ".strtab", 0, 
        bin_len(result), bin_len(contents->strtab->contents), 0, 0, 0, 0,
        section_names_table, mp);
    bin_cat(result, contents->strtab->contents);

    elf64_section_header *comment_header = new_elf64_section_header(
        SECTION_TYPE_PROGBITS, ".comment", 0, 
        bin_len(result), bin_len(contents->comment->contents), 0, 0, 0, 0,
        section_names_table, mp);
    bin_cat(result, contents->comment->contents);

    // we grab the length of the names table before adding the ".shstrtab" entry, hence the +10.
    elf64_section_header *shstrtab_header = new_elf64_section_header(
        SECTION_TYPE_STRTAB, ".shstrtab", 0, 
        bin_len(result), bin_len(section_names_table) + 10, 0, 0, 0, 0,
        section_names_table, mp);
    bin_cat(result, section_names_table);

    section_headers_offset = bin_len(result);
    section_headers_count = 10;
    section_names_table_index = 9;

    // save all section headers
    bin_add_mem(result, empty_section_header, sizeof(elf64_section_header));
    bin_add_mem(result, text_header,          sizeof(elf64_section_header));
    bin_add_mem(result, data_header,          sizeof(elf64_section_header));
    bin_add_mem(result, rodata_header,        sizeof(elf64_section_header));
    bin_add_mem(result, bss_header,           sizeof(elf64_section_header));
    bin_add_mem(result, rela_text_header,     sizeof(elf64_section_header));
    bin_add_mem(result, symtab_header,        sizeof(elf64_section_header));
    bin_add_mem(result, strtab_header,        sizeof(elf64_section_header));
    bin_add_mem(result, comment_header,       sizeof(elf64_section_header));
    bin_add_mem(result, shstrtab_header,      sizeof(elf64_section_header));

    // now that we have offsets and sizes, update program headers
    if (executable) {
        u64 load_headers_mem_address = contents->text->header->virt_address - round_up(file_loading_info_length, 0x1000);
        elf64_prog_header *load_headers_pheader = new_elf64_prog_header(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ, 0x1000,
            0,                        file_loading_info_length, 
            load_headers_mem_address, file_loading_info_length);
        
        elf64_prog_header *text_pheader = new_elf64_prog_header(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_EXECUTE, 0x1000,
            text_header->file_offset,    bin_len(contents->text->contents), 
            contents->text->header->virt_address, bin_len(contents->text->contents));
        
        elf64_prog_header *data_pheader = new_elf64_prog_header(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_WRITE, 0x1000,
            data_header->file_offset,    bin_len(contents->data->contents), 
            contents->data->header->virt_address, bin_len(contents->data->contents));
        
        elf64_prog_header *bss_pheader = new_elf64_prog_header(mp,
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_WRITE, 0x1000,
            bss_header->file_offset,    0,
            contents->bss->header->virt_address, bin_len(contents->bss->contents));
        
        elf64_prog_header *rodata_pheader = new_elf64_prog_header(mp,
            PROG_TYPE_LOAD, PROG_FLAGS_READ, 0x1000,
            rodata_header->file_offset,    bin_len(contents->rodata->contents), 
            contents->rodata->header->virt_address, bin_len(contents->rodata->contents));
        
        bin_seek(result, program_headers_offset);
        bin_write_mem(result, load_headers_pheader, sizeof(elf64_prog_header));
        bin_write_mem(result, text_pheader,         sizeof(elf64_prog_header));
        bin_write_mem(result, data_pheader,         sizeof(elf64_prog_header));
        bin_write_mem(result, bss_pheader,          sizeof(elf64_prog_header));
        bin_write_mem(result, rodata_pheader,       sizeof(elf64_prog_header));
    }

    // now that we have all offsets, update file header
    elf64_header *header = new_elf64_file_header(mp, 
        executable, entry_point,
        program_headers_count, program_headers_offset, 
        section_headers_count, section_headers_offset, section_names_table_index);
    bin_seek(result, 0);
    bin_write_mem(result, header, sizeof(elf64_header));

    return result;
    */

    return NULL;
}   

static obj_symbol *new_obj_symbol(elf64_sym *sym, elf64_section *strtab, llist *sections, mempool *mp) {
    obj_symbol *s = mempool_alloc(mp, sizeof(obj_symbol), "obj_symbol");
    s->name = new_str(mp, "");

    int type = ELF64_ST_TYPE(sym->st_info);
    if (type == STT_SECTION) {  // section names are not contained in the string table
        elf64_section *section = llist_get(sections, sym->st_shndx);
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

static obj_relocation *new_obj_relocation(elf64_rela *rel, elf64_section *symtab, elf64_section *strtab, llist *sections, mempool *mp) {
    obj_relocation *r = mempool_alloc(mp, sizeof(obj_relocation), "obj_relocation");

    // find the symbol this relocation needs to resolve
    size_t sym_index = ELF64_R_SYM(rel->r_info);
    elf64_sym *sym = bin_ptr_at(symtab->contents, sym_index * sizeof(elf64_sym));
    r->symbol_name = new_str(mp, "");

    int sym_type = ELF64_ST_TYPE(sym->st_info);
    if (sym_type == STT_SECTION) {  // section names are not contained in the string table
        elf64_section *section = llist_get(sections, sym->st_shndx);
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

obj_module *unpack_elf64_contents(str *module_name, elf64_contents *contents, mempool *mp) {
    obj_module *module = new_obj_module(mp, str_charptr(module_name));

    elf64_section *text      = elf64_get_section_by_name(contents, new_str(mp, ".text"));
    elf64_section *data      = elf64_get_section_by_name(contents, new_str(mp, ".data"));
    elf64_section *bss       = elf64_get_section_by_name(contents, new_str(mp, ".bss"));
    elf64_section *rodata    = elf64_get_section_by_name(contents, new_str(mp, ".rodata"));
    elf64_section *rela_text = elf64_get_section_by_name(contents, new_str(mp, ".rela.text"));
    elf64_section *symtab    = elf64_get_section_by_name(contents, new_str(mp, ".symtab"));
    elf64_section *strtab    = elf64_get_section_by_name(contents, new_str(mp, ".strtab"));

    if (text   != NULL) bin_cpy(module->text->contents,   text->contents);
    if (data   != NULL) bin_cpy(module->data->contents,   data->contents);
    if (bss    != NULL) bin_cpy(module->bss->contents,    bss->contents);
    if (rodata != NULL) bin_cpy(module->rodata->contents, rodata->contents);
    
    // unpack symbols into the relevant sections
    if (symtab != NULL && strtab != NULL) {
        int num_symbols = bin_len(symtab->contents) / sizeof(elf64_sym);
        elf64_sym *sym;
        for (int i = 0; i < num_symbols; i++) {
            sym = (elf64_sym *)bin_ptr_at(symtab->contents, i * sizeof(elf64_sym));
            obj_symbol *s = new_obj_symbol(sym, strtab, contents->sections, mp);
            // must find the relevant section (grab the indexes of the four main sections)
            llist_add(module->text->symbols, s);
        }
    }
    
    // unpack relocations into relevant sections
    if (rela_text != NULL && symtab != NULL && strtab != NULL) {
        int num_relocations = bin_len(rela_text->contents) / sizeof(elf64_rela);
        elf64_rela *rela;
        for (int i = 0; i < num_relocations; i++) {
            rela = (elf64_rela *)bin_ptr_at(rela_text->contents, i * sizeof(elf64_rela));
            obj_relocation *r = new_obj_relocation(rela, symtab, strtab, contents->sections, mp);

            llist_add(module->text->relocations, r);  // must find the relevant list to add this to.
        }
    }

    return module;
}

static void print_obj_section(obj_section *s, FILE *f) {
    mempool *scratch = new_mempool();
    fprintf(f, "  Section %s\n", str_charptr(s->name));

    if (bin_len(s->contents) > 0) {
        size_t bytes = bin_len(s->contents) > 64 ? 64 : bin_len(s->contents);
        fprintf(f, "    Contents (%lu / %lu total bytes)\n", bytes, bin_len(s->contents));
        bin_print_hex(s->contents, 6, 0, bytes, f);
    }

    if (llist_length(s->symbols) > 0) {
        fprintf(f, "    Symbols\n");
        fprintf(f, "      Num    Value     Size  Scope   Name\n");
        //                Num    Value     Size  Scope   Name
        //                123 12345678 12345678  GLOBAL  123456789...
        iterator *symbols = llist_create_iterator(s->symbols, scratch);
        int num = 0;
        for_iterator(obj_symbol, s, symbols) {
            fprintf(f, "      %3d %08lx %8lu  %-6s  %s\n",
                num++,
                s->value,
                s->size,
                s->global ? "GLOBAL" : "LOCAL",
                str_charptr(s->name));
        }
    }

    if (llist_length(s->relocations)) {
        fprintf(f, "    Relocations\n");
        fprintf(f, "        Offset  Type  Addendum  Symbol\n");
        //         "        Offset  Type  Addendum  Symbol
        //         "      12345678  1234  12345678  12345...
        iterator *relocations = llist_create_iterator(s->relocations, scratch);
        for_iterator(obj_relocation, r, relocations) {
            fprintf(f, "      %08lx  %4d  %+8ld  %s\n",
                r->offset,
                r->type,
                r->addendum,
                str_charptr(r->symbol_name));
        }
    }

    mempool_release(scratch);
}

void print_obj_module(obj_module *module, FILE *f) {
    // print each section with it's symbols and relocations
    fprintf(f, "Module %s\n", str_charptr(module->name));

    print_obj_section(module->text, f);
    print_obj_section(module->data, f);
    print_obj_section(module->bss, f);
    print_obj_section(module->rodata, f);
}
