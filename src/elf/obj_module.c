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
    module->module_name = new_str(mp, name);
    module->text = new_obj_section(mp, ".text");
    module->data = new_obj_section(mp, ".data");
    module->bss = new_obj_section(mp, ".bss");
    module->rodata = new_obj_section(mp, "rodata");
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
    add_elf64_symbol(module->module_name, 0, 0, STT_FILE, false, SHN_ABS, c->symtab, c->strtab);

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

static obj_symbol *new_obj_symbol(mempool *mp) {
    obj_symbol *s = mempool_alloc(mp, sizeof(obj_symbol), "obj_symbol");

    // ...

    return s;
}

static obj_relocation *new_obj_relocation(mempool *mp) {
    obj_relocation *r = mempool_alloc(mp, sizeof(obj_relocation), "obj_relocation");

    // ...

    return r;
}

obj_module *unpack_elf64_contents(str *module_name, elf_contents2 *contents, mempool *mp) {
    obj_module *module = new_obj_module(mp, str_charptr(module_name));

    bin_cat(module->text->contents, contents->text->contents);
    bin_cat(module->data->contents, contents->data->contents);
    bin_cat(module->bss->contents, contents->bss->contents);
    bin_cat(module->rodata->contents, contents->rodata->contents);

    // unpack symbols into the relevant index...
    
     
    // unpack relocations

    return module;
}



