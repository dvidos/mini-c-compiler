#include <string.h>
#include "obj_module.h"
#include "elf_format.h"

static obj_section *new_obj_section(mempool *mp, const char *name) {
    obj_section *s = mempool_alloc(mp, sizeof(obj_section), "obj_section");
    s->name = new_str(mp, name);
    s->mem_address = 0;
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

static size_t get_strtab_index(str *s, elf_contents2_section *strtab) {
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

static void add_elf64_symbol(str *name, size_t value, size_t size, int st_type, bool global, int st_shndx, elf_contents2_section *symtab, elf_contents2_section *strtab) {
    elf64_sym sym;
    sym.st_name = get_strtab_index(name, strtab);
    sym.st_value = value;
    sym.st_size = size;
    sym.st_info = ELF64_ST_INFO(global ? STB_GLOBAL : STB_LOCAL, st_type);
    sym.st_other = 0;
    sym.st_shndx = st_shndx;
    bin_add_mem(symtab->contents, &sym, sizeof(elf64_sym));
}

static void pack_elf64_symbols(llist *symbols_list, int st_type, int st_shndx, mempool *mp, elf_contents2_section *symtab, elf_contents2_section *strtab) {
    iterator *it = llist_create_iterator(symbols_list, mp);
    for_iterator(obj_symbol, s, it)
        add_elf64_symbol(s->name, s->value, s->size, st_type, s->global, st_shndx, symtab, strtab);
}

static int find_elf64_symbol(const char *name, elf_contents2_section *symtab, elf_contents2_section *strtab) {
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

static int elf64_symbols_table_len(elf_contents2_section *symtab) {
    return bin_len(symtab->contents) / sizeof(elf64_sym);
};

static void pack_elf64_relocations(llist *relocations_list, mempool *mp, elf_contents2_section *relatab, elf_contents2_section *symtab, elf_contents2_section *strtab) {
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

elf_contents2 *pack_elf64_contents(obj_module *module, mempool *mp) {
    elf_contents2 *c = new_elf_contents2(mp);
    iterator *it;

    // copy the base segments contents
    bin_cat(c->text->contents, module->text->contents);
    bin_cat(c->data->contents, module->data->contents);
    bin_cat(c->bss->contents, module->bss->contents);
    bin_cat(c->rodata->contents, module->rodata->contents);

    // start populating symbols and relocations
    mempool *scratch = new_mempool();

    // add the empty symbol to the symbol table
    add_elf64_symbol(new_str(mp, ""), 0, 0, STT_NOTYPE, false, SHN_UNDEF, c->symtab, c->strtab);

    // add symbol for the file name
    add_elf64_symbol(module->name, 0, 0, STT_FILE, false, SHN_ABS, c->symtab, c->strtab);

    // add symbol for each section
    add_elf64_symbol(module->text->name, 0, 0, STT_SECTION, false, ELF_TEXT_SHNDX, c->symtab, c->strtab);
    add_elf64_symbol(module->data->name, 0, 0, STT_SECTION, false, ELF_TEXT_SHNDX, c->symtab, c->strtab);
    add_elf64_symbol(module->bss->name, 0, 0, STT_SECTION, false, ELF_TEXT_SHNDX, c->symtab, c->strtab);
    add_elf64_symbol(module->rodata->name, 0, 0, STT_SECTION, false, ELF_TEXT_SHNDX, c->symtab, c->strtab);

    // pack various symbols into songle symtab section (and names into strtab)
    pack_elf64_symbols(module->text->symbols,   STT_FUNC,   ELF_TEXT_SHNDX,   scratch, c->symtab, c->strtab);
    pack_elf64_symbols(module->data->symbols,   STT_OBJECT, ELF_DATA_SHNDX,   scratch, c->symtab, c->strtab);
    pack_elf64_symbols(module->bss->symbols,    STT_OBJECT, ELF_BSS_SHNDX,    scratch, c->symtab, c->strtab);
    pack_elf64_symbols(module->rodata->symbols, STT_OBJECT, ELF_RODATA_SHNDX, scratch, c->symtab, c->strtab);

    // pack relocations as well
    pack_elf64_relocations(module->text->relocations, scratch, c->rela_text, c->symtab, c->strtab);
    pack_elf64_relocations(module->data->relocations, scratch, c->rela_data, c->symtab, c->strtab);

    mempool_release(scratch);
    return c;
}

static obj_symbol *new_obj_symbol(elf64_sym *sym, elf_contents2_section *strtab, mempool *mp) {
    obj_symbol *s = mempool_alloc(mp, sizeof(obj_symbol), "obj_symbol");

    s->name = bin_str(strtab->contents, sym->st_name, mp);
    s->global = ELF64_ST_BIND(sym->st_info) == STB_GLOBAL;
    s->value = sym->st_value;
    s->size = sym->st_size;

    return s;
}

static obj_relocation *new_obj_relocation(elf64_rela *rel, elf_contents2_section *symtab, elf_contents2_section *strtab, mempool *mp) {
    obj_relocation *r = mempool_alloc(mp, sizeof(obj_relocation), "obj_relocation");

    // find the symbol this relocation needs to resolve
    size_t sym_index = ELF64_R_SYM(rel->r_info);
    elf64_sym *sym = bin_ptr_at(symtab->contents, sym_index * sizeof(elf64_sym));

    r->symbol_name = bin_str(strtab->contents, sym->st_name, mp);
    r->offset = rel->r_offset;
    r->addendum = rel->r_addend;
    r->type = ELF64_R_TYPE(rel->r_info);

    return r;
}

obj_module *unpack_elf64_contents(str *module_name, elf_contents2 *contents, mempool *mp) {
    obj_module *module = new_obj_module(mp, str_charptr(module_name));

    bin_cat(module->text->contents, contents->text->contents);
    bin_cat(module->data->contents, contents->data->contents);
    bin_cat(module->bss->contents, contents->bss->contents);
    bin_cat(module->rodata->contents, contents->rodata->contents);

    // unpack symbols into the relevant index...
    int num_symbols = bin_len(contents->symtab->contents) / sizeof(elf64_sym);
    elf64_sym *sym;
    for (int i = 0; i < num_symbols; i++) {
        sym = (elf64_sym *)bin_ptr_at(contents->symtab->contents, i * sizeof(elf64_sym));
        obj_symbol *s = new_obj_symbol(sym, contents->strtab, mp);
        // must find the relevant section
        llist_add(module->text->symbols, s);
    }
    
    // unpack relocations
    int num_relocations = bin_len(contents->rela_text->contents) / sizeof(elf64_rela);
    elf64_rela *rela;
    for (int i = 0; i < num_relocations; i++) {
        rela = (elf64_rela *)bin_ptr_at(contents->rela_text->contents, i * sizeof(elf64_rela));
        obj_relocation *r = new_obj_relocation(rela, contents->symtab, contents->strtab, mp);        
        llist_add(module->text->relocations, r);  // must find the relevant list to add this to.
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
        fprintf(f, "      Num    Value     Size  Vis     Name\n");
        //                Num    Value     Size  Vis     Name
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
