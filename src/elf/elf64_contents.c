#include <unistd.h>
#include <string.h>
#include "elf_format.h"
#include "elf64_contents.h"


static elf64_section *elf64_contents_create_section(elf64_contents *contents, str *name, size_t type);
static elf64_prog_header *elf64_contents_create_prog_header(elf64_contents *contents);
static void elf64_contents_add_section(elf64_contents *contents, elf64_section *s);
static void elf64_contents_add_prog_header(elf64_contents *contents, elf64_prog_header *p);
static elf64_section *elf64_contents_get_section_by_name(elf64_contents *contents, str *name);
static elf64_section *elf64_contents_get_section_by_index(elf64_contents *contents, int index);
static elf64_section *elf64_contents_get_section_by_type(elf64_contents *contents, int type);
static void elf64_contents_print(elf64_contents *contents, FILE *stream);
static bool elf64_contents_save(elf64_contents *contents, str *filename);

struct elf64_contents_ops elf64_contents_ops = { 
    .create_section = elf64_contents_create_section,
    .create_prog_header = elf64_contents_create_prog_header,
    .add_section = elf64_contents_add_section,
    .add_prog_header = elf64_contents_add_prog_header,
    .get_section_by_name = elf64_contents_get_section_by_name,
    .get_section_by_index = elf64_contents_get_section_by_index,
    .get_section_by_type = elf64_contents_get_section_by_type,
    .print = elf64_contents_print,
    .save = elf64_contents_save,
};

static void elf64_section_add_symbol(elf64_section *s, size_t name_offset, size_t value, size_t size, int type, int binding, int section_index);
static void elf64_section_add_relocation(elf64_section *s, size_t offset, size_t symbol_no, size_t type, long addendum);
static size_t elf64_section_add_strz_get_offset(elf64_section *s, str *string);
static int elf64_section_find_named_symbol(elf64_section *s, str *name, elf64_section *strtab);
static void elf64_section_add_named_symbol(elf64_section *s, str *name, size_t value, size_t size, int type, int binding, int section_index, elf64_section *strtab);
static void elf64_section_add_named_relocation(elf64_section *s, size_t offset, str *symbol_name, size_t type, long addend, elf64_section *symtab, elf64_section *strtab);
static void elf64_section_print(elf64_section *s, FILE *stream);
static int elf64_section_count_symbols(elf64_section *s);
static void elf64_section_print_symbol(elf64_section *s, int symbol_no, elf64_section *strtab, FILE *stream);
static int elf64_section_count_relocations(elf64_section *s);
static void elf64_section_print_relocation(elf64_section *s, int rel_no, elf64_section *symtab, elf64_section *strtab, FILE *stream);

struct elf64_section_ops elf64_section_ops = {
    .add_symbol = elf64_section_add_symbol,
    .add_relocation = elf64_section_add_relocation,
    .add_strz_get_offset = elf64_section_add_strz_get_offset,
    .find_named_symbol = elf64_section_find_named_symbol,
    .add_named_symbol = elf64_section_add_named_symbol,
    .add_named_relocation = elf64_section_add_named_relocation,
    .print = elf64_section_print,
    .count_symbols = elf64_section_count_symbols,
    .print_symbol = elf64_section_print_symbol,
    .count_relocations = elf64_section_count_relocations,
    .print_relocation = elf64_section_print_relocation,
};


static bin *flatten_elf64_contents(elf64_contents *contents, mempool *mp);

// --------------------------------------------------

static elf64_section *elf64_contents_create_section(elf64_contents *contents, str *name, size_t type) {
    elf64_section *s = mempool_alloc(contents->mempool, sizeof(elf64_section), "elf64_section");

    s->index = 0;
    s->name = name;
    s->header = mempool_alloc(contents->mempool, sizeof(elf64_section_header), "elf64_section_header");
    s->header->type = type;
    s->contents = new_bin(contents->mempool);
    s->ops = &elf64_section_ops;

    return s;
}

static elf64_prog_header *elf64_contents_create_prog_header(elf64_contents *contents) {
    elf64_prog_header *h = mempool_alloc(contents->mempool, sizeof(elf64_prog_header), "elf64_prog_header");
    memset(h, 0, sizeof(elf64_prog_header));
    return h;
}

static void elf64_contents_add_section(elf64_contents *contents, elf64_section *s) {
    s->index = llist_length(contents->sections);
    llist_add(contents->sections, s);
}

static void elf64_contents_add_prog_header(elf64_contents *contents, elf64_prog_header *p) {
    llist_add(contents->prog_headers, p);
}

static elf64_section *elf64_contents_get_section_by_name(elf64_contents *contents, str *name) {
    for_list(contents->sections, elf64_section, s) {
        if (str_equals(s->name, name))
            return s;
    }
    return NULL;
}

static elf64_section *elf64_contents_get_section_by_index(elf64_contents *contents, int index) {
    for_list(contents->sections, elf64_section, s) {
        if (s->index == index)
            return s;
    }
    return NULL;
}

static elf64_section *elf64_contents_get_section_by_type(elf64_contents *contents, int type) {
    for_list(contents->sections, elf64_section, s) {
        if (s->header->type == type)
            return s;
    }
    return NULL;
}

static void elf64_contents_print(elf64_contents *contents, FILE *stream) {
    char *prog_types[] = { "NULL", "LOAD", "DYNAMIC", "INTERP", "NOTE", "SHLIB", "PHDR", "TLS" };

    fprintf(stream, "ELF64 Contents (magic = %c%c%c%c)\n", 
        contents->header->identity[0], contents->header->identity[1], contents->header->identity[2], contents->header->identity[3]);
    fprintf(stream, "  Type: %d (1=relocatable, 2=executable, 3=dynamic executable)\n", contents->header->file_type);
    
    if (llist_length(contents->prog_headers) > 0) {
        fprintf(stream, "  Program headers\n");
        fprintf(stream, "    Type         Offset    Virt Addr    Phys Addr  FileSize   MemSiz Flg Align\n");
        //              "    1234567890 12345678 123456789012 123456789012  12345678 12345678 XXX 12345678"
        for_list(contents->prog_headers, elf64_prog_header, h) {
            fprintf(stream, "    %-10s %08lx %012lx %012lx  %08lx %08lx %c%c%c %ld\n",
                h->type >= 0 && h->type < (sizeof(prog_types)/sizeof(prog_types[0])) ? prog_types[h->type] : "?",
                h->file_offset,
                h->virt_address,
                h->phys_address,
                h->file_size,
                h->memory_size,
                h->flags & PROG_FLAGS_READ ? 'R' : ' ',
                h->flags & PROG_FLAGS_WRITE ? 'W' : ' ',
                h->flags & PROG_FLAGS_EXECUTE ? 'X' : ' ',
                h->align
            );
        }
    } else {
        fprintf(stream, "  No program headers present\n");
    }

    fprintf(stream, "  Section headers\n");
    elf64_section_print(NULL, stream);
    for_list(contents->sections, elf64_section, s)
        s->ops->print(s, stream);

    // elf64_section *strtab = elf64_contents_get_section_by_name(contents, new_str(contents->mempool, ".strtab"));
    // elf64_section *symtab = elf64_contents_get_section_by_name(contents, new_str(contents->mempool, ".symtab"));
    // if (strtab != NULL && symtab != NULL) {
    //     fprintf(stream, "  Symbols\n");
    //     int symbols_count = symtab->ops->count_symbols(symtab);
    //     for (int i = 0; i < symbols_count; i++)
    //         symtab->ops->print_symbol(symtab, i, strtab, stream);
    // }
}

static bool elf64_contents_save(elf64_contents *contents, str *filename) {
    bin *file_contents = flatten_elf64_contents(contents, contents->mempool);
    return bin_save_to_file(file_contents, filename);
}


// ------------------------------------------------------------

static void elf64_section_add_symbol(elf64_section *s, size_t name_offset, size_t value, size_t size, int type, int binding, int section_index) {
    elf64_sym sym;
    sym.st_name = name_offset;
    sym.st_value = value;
    sym.st_size = size;
    sym.st_info = ELF64_ST_INFO(binding, type);
    sym.st_other = 0;
    sym.st_shndx = section_index;
    bin_add_mem(s->contents, &sym, sizeof(elf64_sym));
}

static void elf64_section_add_relocation(elf64_section *s, size_t offset, size_t symbol_no, size_t type, long addendum) {
    elf64_rela rela;
    rela.r_offset = offset;
    rela.r_info = ELF64_R_INFO(symbol_no, type);
    rela.r_addend = addendum;
    bin_add_mem(s->contents, &rela, sizeof(elf64_rela));
}

static size_t elf64_section_add_strz_get_offset(elf64_section *s, str *string) {
    // first byte is always zero to allow empty strings
    if (bin_len(s->contents) == 0)
        bin_add_byte(s->contents, 0x00);

    if (str_len(string) == 0)
        return 0;
    
    // in case the name already exists, reuse it
    int index = bin_index_of(s->contents, str_charptr(string), str_len(string) + 1);
    if (index >= 0)
        return index;

    // otherwise add it, saving address first
    index = bin_len(s->contents);
    bin_add_str(s->contents, string);

    return index;
}

static int elf64_section_find_named_symbol(elf64_section *s, str *name, elf64_section *strtab) {
    // we could speed things up with a hashtable
    elf64_sym *sym_ptr;
    char *name_ptr;

    int total_syms = bin_len(s->contents) / sizeof(elf64_sym);
    for (int i = 0; i < total_syms; i++) {
        sym_ptr = bin_ptr_at(s->contents, i * sizeof(elf64_sym));
        char *name_ptr = bin_ptr_at(strtab->contents, sym_ptr->st_name);
        if (str_cmps(name, name_ptr) == 0)
            return i;
    }
    return -1;
}

static void elf64_section_add_named_symbol(elf64_section *s, str *name, size_t value, size_t size, int type, int binding, int section_index, elf64_section *strtab) {
    size_t name_offset = strtab->ops->add_strz_get_offset(strtab, name);
    elf64_section_add_symbol(s, name_offset, value, size, type, binding, section_index);
}

static void elf64_section_add_named_relocation(elf64_section *s, size_t offset, str *symbol_name, size_t type, long addend, elf64_section *symtab, elf64_section *strtab) {
    int symbol_no = elf64_section_find_named_symbol(symtab, symbol_name, strtab);
    if (symbol_no == -1) {
        elf64_section_add_named_symbol(symtab, symbol_name, 0, 0, STT_NOTYPE, STB_GLOBAL, SHN_UNDEF, strtab);
        symbol_no = elf64_section_find_named_symbol(symtab, symbol_name, strtab);
    }
    elf64_section_add_relocation(s, offset, symbol_no, type, addend);
}

static void elf64_section_print(elf64_section *s, FILE *stream) {
    char *section_types[] = { "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NUM" };    

    if (s == NULL) {
        fprintf(stream, "    Num Name             Type        Address   Offset     Size Flg Lk Inf Al  Cont\n");
        //              "    123 1234567890123456 12345678 1234567890 12345678 12345678 123 12 123 12  1234"
    } else {
        fprintf(stream, "    %3d %-16s %-8s %010lx %08lx %08lx %c%c%c %2d %3d %2ld  %4ld\n",
            s->index,
            str_charptr(s->name),
            s->header->type >= 0 && s->header->type < (sizeof(section_types)/sizeof(section_types[0])) ? section_types[s->header->type] : "?",
            s->header->virt_address,
            s->header->file_offset,
            s->header->size,
            s->header->flags & SECTION_FLAGS_ALLOC ? 'A' : ' ',
            s->header->flags & SECTION_FLAGS_WRITE ? 'W' : ' ',
            s->header->flags & SECTION_FLAGS_EXECINSTR ? 'X' : ' ',
            s->header->link,
            s->header->info,
            s->header->address_alignment,
            s->contents == NULL ? 0L : bin_len(s->contents)
        );
    }
}

static int elf64_section_count_symbols(elf64_section *s) {
    return bin_len(s->contents) / sizeof(elf64_sym);
}

static void elf64_section_print_symbol(elf64_section *s, int symbol_no, elf64_section *strtab, FILE *stream) {
}

static int elf64_section_count_relocations(elf64_section *s) {
    return bin_len(s->contents) / sizeof(elf64_sym);
}

static void elf64_section_print_relocation(elf64_section *s, int rel_no, elf64_section *symtab, elf64_section *strtab, FILE *stream) {
}

// -------------------------------------------

static inline size_t round_up(size_t value, size_t granularity) {
    return (((value + granularity - 1) / granularity) * granularity);
}

static elf64_header *new_elf64_file_header(mempool *mp, bool executable, u64 entry_point) {
    elf64_header *h = mempool_alloc(mp, sizeof(elf64_header), "elf64_header");

    memcpy(h->identity, "\177ELF", 4);
    h->identity[ELF_IDENTITY_CLASS] = ELF_CLASS_64;
    h->identity[ELF_IDENTITY_DATA] = ELF_DATA2_LSB;
    h->identity[ELF_IDENTITY_VERSION] = ELF_VERSION_CURRENT;
    h->identity[ELF_IDENTITY_OS_ABI] = ELF_OSABI_SYSV;
    h->file_type = executable ? ELF_TYPE_EXEC : ELF_TYPE_REL;
    h->version = ELF_VERSION_CURRENT;
    h->entry_point = entry_point;
    h->machine = ELF_MACHINE_X86_64;
    h->elf_header_size = sizeof(elf64_header);

    h->prog_headers_entries = 0;
    h->prog_headers_entry_size = sizeof(elf64_prog_header);
    h->prog_headers_offset = 0;

    h->section_headers_entries = 0;
    h->section_headers_entry_size = sizeof(elf64_section_header);
    h->section_headers_offset = 0;
    h->section_headers_strings_entry = 0;

    return h;
}

static elf64_section_header *new_elf64_section_header(int type, char *name, u64 flags, u64 file_offset, u64 size, u64 item_size, u64 link, u64 info, u64 virt_address, bin *sections_names_table, mempool *mp) {
    elf64_section_header *h = mempool_alloc(mp, sizeof(elf64_section_header), "elf64_section_header");

    u64 name_offset = bin_len(sections_names_table);
    bin_add_mem(sections_names_table, name, strlen(name) + 1);

    h->type = type;
    h->flags = flags;
    h->name = name_offset;
    h->file_offset = file_offset;
    h->virt_address = virt_address;
    h->size = size;
    h->entry_size = item_size;
    h->link = link;
    h->info = info;
    h->address_alignment = 8;

    return h;
}

static elf64_prog_header *generate_prog_header(mempool *mp, size_t file_offset, size_t file_size, size_t mem_address, size_t mem_size, unsigned flags, unsigned align) {
    elf64_prog_header *prog = mempool_alloc(mp, sizeof(elf64_prog_header), "elf64_prog_header");
    memset(prog, 0, sizeof(elf64_prog_header));

    prog->type = PROG_TYPE_LOAD;
    prog->file_offset = file_offset;
    prog->file_size = file_size;
    prog->phys_address = mem_address;
    prog->virt_address = mem_address;
    prog->memory_size = mem_size;
    prog->flags = flags;
    prog->align = align;

    return prog;
}

static void generate_program_headers(elf64_contents *contents) {
    // go over sections, see what should be loaded (and how) and create needed program headers
    mempool *scratch = new_mempool();
    size_t curr_filepos = sizeof(elf64_header);
    size_t prog_file_offset;
    size_t prog_file_size;
    size_t prog_memory_address;
    size_t prog_memory_size;
    int prog_flags = -1;
    int align = 4096;

    llist_clear(contents->prog_headers);

    // keep a running group, group similar sections together
    str *last_group_key = new_str(scratch, NULL);
    for_list(contents->sections, elf64_section, sect) {
        if (sect->header->type != SECTION_TYPE_PROGBITS && sect->header->type != SECTION_TYPE_NOBITS) {
            curr_filepos += bin_len(sect->contents);
            continue;
        }
        
        str *prog_group_key = new_strf(scratch, "%s-%s%s%s",
            sect->header->type == SECTION_TYPE_PROGBITS ? "PROGBITS" : "NOBITS",
            sect->header->flags & SECTION_FLAGS_ALLOC ? "A" : "-",
            sect->header->flags & SECTION_FLAGS_WRITE ? "W" : "-",
            sect->header->flags & SECTION_FLAGS_EXECINSTR ? "X" : "-");

        // new group, finish last if applicable
        if (!str_equals(prog_group_key, last_group_key)) {
            if (!str_is_empty(last_group_key)) {
                // add prev program header
                llist_add(contents->prog_headers, generate_prog_header(contents->mempool,
                    prog_file_offset, prog_file_size, prog_memory_address, prog_memory_size, prog_flags, align));
            }
            
            // new program header, reset group totals
            prog_file_offset = curr_filepos;
            prog_file_size = 0;
            prog_memory_address = sect->header->virt_address;
            prog_memory_size = 0;
            prog_flags = (sect->header->flags & SECTION_FLAGS_ALLOC     ? PROG_FLAGS_READ    : 0) |
                    (sect->header->flags & SECTION_FLAGS_WRITE     ? PROG_FLAGS_WRITE   : 0) |
                    (sect->header->flags & SECTION_FLAGS_EXECINSTR ? PROG_FLAGS_EXECUTE : 0); 
            str_cpy(last_group_key, prog_group_key);
        }

        // update group totals
        prog_file_size += sect->header->type == SECTION_TYPE_NOBITS ? 0 : bin_len(sect->contents);
        prog_memory_size += bin_len(sect->contents);

        // update running values
        curr_filepos += bin_len(sect->contents);
    }

    if (!str_is_empty(last_group_key)) {
        // finish last group as applicable
        llist_add(contents->prog_headers, generate_prog_header(contents->mempool,
            prog_file_offset, prog_file_size, prog_memory_address, prog_memory_size, prog_flags, align));
    }
}

static bin *flatten_elf64_contents(elf64_contents *contents, mempool *mp) {
    /*
        File structure is as follows
        ----------------------------------------
        [ elf file header      ]
        [ section 1   contents ]  <-- these are text, data, bss, etc contents
        [ section 2   contents ]
        [ section ... contents ]
        [ section 0   header   ]  <-- section headers
        [ section 1   header   ]
        [ section ... header   ]
        [ program header 0     ]  <-- skipped for .o files
        [ program header 1     ]
        [ program header ...   ]
    */

    // recalculate sections location and size, to make sure our program headers are accurate
    size_t file_offset = sizeof(elf64_header);
    for_list(contents->sections, elf64_section, sect) {
        sect->header->file_offset = file_offset;
        sect->header->size = bin_len(sect->contents);
        file_offset += bin_len(sect->contents);
    }

    // if we are creating an executable, calculate the program headers now
    // this could not happen from the obj_module, it does not know file offsets
    if (contents->header->file_type == ELF_TYPE_EXEC) {
        generate_program_headers(contents);
        contents->ops->print(contents, stdout);
    }

    bin *file_data = new_bin(mp);
    bin_add_mem(file_data, contents->header, sizeof(elf64_header));

    // keep note where the section headers will be
    contents->header->section_headers_entry_size = sizeof(elf64_section_header);
    contents->header->section_headers_entries = llist_length(contents->sections);

    // save curr offset and prog headers stuff
    contents->header->prog_headers_entry_size = sizeof(elf64_prog_header);
    contents->header->prog_headers_entries = llist_length(contents->prog_headers);

    // hope caller has created the section header names
    contents->header->section_headers_strings_entry = llist_length(contents->sections) - 1;

    // write all section contents
    for_list(contents->sections, elf64_section, s) {
        s->header->file_offset = bin_len(file_data);
        s->header->size = (s->header->type == SECTION_TYPE_NOBITS) ? 0 : bin_len(s->contents);
        
        // contents of NOBITS sections should not be saved
        if (s->header->type == SECTION_TYPE_NOBITS)
            continue;

        bin_cat(file_data, s->contents);
    }

    // write all section headers
    contents->header->section_headers_offset = llist_length(contents->sections) == 0 ? 0 : bin_len(file_data);
    for_list(contents->sections, elf64_section, s)
        bin_add_mem(file_data, s->header, sizeof(elf64_section_header));
    
    // write all program headers
    contents->header->prog_headers_offset = llist_length(contents->prog_headers) == 0 ? 0 : bin_len(file_data);
    for_list(contents->prog_headers, elf64_prog_header, h)
        bin_add_mem(file_data, h, sizeof(elf64_prog_header));

    // find entry point, if applicable
    

    // finally, rewrite header, to update the offsets of the various tables
    bin_seek(file_data, 0);
    bin_write_mem(file_data, contents->header, sizeof(elf64_header));

    // done
    return file_data;
}


// -------------------------------------------

elf64_contents *new_elf64_contents(mempool *mp) {
    elf64_contents *c = mempool_alloc(mp, sizeof(elf64_contents), "elf64_contents");
    c->header = new_elf64_file_header(mp, false, 0);
    c->sections = new_llist(mp);
    c->prog_headers = new_llist(mp);
    c->ops = &elf64_contents_ops;
    c->mempool = mp;
    return c;
}

elf64_contents *new_elf64_contents_from_binary(mempool *mp, bin *buffer) {
    if (buffer == NULL)
        return NULL;

    mempool *scratch = new_mempool();
    elf64_contents *contents = new_elf64_contents(mp);
    elf64_section *s;

    // extract header
    bin_seek(buffer, 0);
    bin_read_mem(buffer, contents->header, sizeof(elf64_header));

    // some validations
    if (memcmp(contents->header->identity, "\177ELF", 4) != 0)
        return false;
    if (contents->header->identity[ELF_IDENTITY_CLASS] != ELF_CLASS_64)
        return false;
    if (contents->header->file_type != ELF_TYPE_REL)
        return false;

    // let's load all the sections, headers and bodies (we don't load programs)
    for (int i = 0; i < contents->header->section_headers_entries; i++) {
        s = elf64_contents_create_section(contents, new_str(mp, ""), 0);
        s->index = i;

        // header
        bin_seek(buffer, contents->header->section_headers_offset + i * sizeof(elf64_section_header));
        bin_read_mem(buffer, s->header, sizeof(elf64_section_header));
        // contents
        bin_cpy(s->contents, bin_slice(buffer, s->header->file_offset, s->header->size, scratch));

        llist_add(contents->sections, s);
    }

    // now that we have all section headers, we can assign headers names
    bin *names_table = NULL;
    if (contents->header->section_headers_strings_entry > 0) {
        elf64_section *shstrtab = llist_get(contents->sections, contents->header->section_headers_strings_entry);
        names_table = shstrtab->contents;
    }
    if (names_table != NULL) {
        for (int i = 0; i < contents->header->section_headers_entries; i++) {
            elf64_section *s = llist_get(contents->sections, i);
            if (s->header->name == 0)
                continue;
            
            str_cpy(s->name, bin_str(names_table, s->header->name, scratch));
        }
    }
    
    mempool_release(scratch);
    return contents;
}

// -----------------------------------------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
static void elf_unit_test_obj_file(mempool *mp);
static void elf_unit_test_executable_file(mempool *mp);

void elf_unit_tests() {
    mempool *mp = new_mempool();

    elf64_contents *c = new_elf64_contents(mp);
    assert(c != NULL);
    assert(c->header != NULL);
    assert(c->prog_headers != NULL);
    assert(c->sections != NULL);

    elf_unit_test_obj_file(mp);

    mempool_release(mp);
}

static void elf_unit_test_obj_file(mempool *mp) {
    char buffer[64];
    str *obj_filename = new_str(mp, "temp_unit_test_d06ffeae-d503-45ea-868e-9e36b73d1f69.o");

    // save and load an obj file.
    elf64_contents *contents = new_elf64_contents(mp);
    elf64_section *s;

    s = elf64_contents_create_section(contents, new_str(mp, ".text"), SECTION_TYPE_PROGBITS);
    bin_pad(s->contents, 'T', 64);
    llist_add(contents->sections, s);

    s = elf64_contents_create_section(contents, new_str(mp, ".data"), SECTION_TYPE_PROGBITS);
    bin_pad(s->contents, 'D', 64);
    llist_add(contents->sections, s);

    s = elf64_contents_create_section(contents, new_str(mp, ".bss"), SECTION_TYPE_NOBITS);
    bin_pad(s->contents, 'B', 64);
    llist_add(contents->sections, s);

    s = elf64_contents_create_section(contents, new_str(mp, ".rodata"), SECTION_TYPE_PROGBITS);
    bin_pad(s->contents, 'R', 64);
    llist_add(contents->sections, s);

    bool elf_obj_saved = elf64_contents_save(contents, obj_filename);
    assert(elf_obj_saved);

    // pause and check using `readelf`
    // assume time passes ... (which it always does)

    bin *file_data = new_bin_from_file(mp, obj_filename);
    elf64_contents *loaded = new_elf64_contents_from_binary(mp, file_data);
    unlink(str_charptr(obj_filename));
    assert(loaded != NULL);
    assert(llist_length(loaded->sections) >= 4);

    s = contents->ops->get_section_by_name(contents, new_str(mp, ".text"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "TTTTTTTT", 8) == 0);

    s = contents->ops->get_section_by_name(contents, new_str(mp, ".data"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "DDDDDDDD", 8) == 0);

    // bss is not written, nor read
    s = contents->ops->get_section_by_name(contents, new_str(mp, ".rodata"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "RRRRRRRR", 8) == 0);
}
#endif // INCLUDE_UNIT_TESTS

