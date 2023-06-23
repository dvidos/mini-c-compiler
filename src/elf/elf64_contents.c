#include <unistd.h>
#include <string.h>
#include "elf_format.h"
#include "elf64_contents.h"

#define ALIGN_SIZE  0x1000

// -------------------------------------------

elf64_contents *new_elf64_contents(mempool *mp) {
    elf64_contents *c = mempool_alloc(mp, sizeof(elf64_contents), "elf64_contents");

    c->header = new_elf64_file_header(mp, false, 0);
    c->sections = new_llist(mp);
    c->prog_headers = new_llist(mp);
    c->mempool = mp;

    return c;
}

elf64_contents *new_elf64_contents_from_binary(mempool *mp, bin *buffer) {
    mempool *scratch = new_mempool();
    elf64_section *s;

    if (buffer == NULL)
        return NULL;

    elf64_contents *contents = new_elf64_contents(mp);

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
        s = new_elf64_section(mp);
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

elf64_section *new_elf64_section(mempool *mp) {
    elf64_section *s = mempool_alloc(mp, sizeof(elf64_section), "elf64_section");

    s->index = 0;
    s->header = mempool_alloc(mp, sizeof(elf64_section_header), "elf64_section_header");
    s->contents = new_bin(mp);
    s->name = new_str(mp, "");

    return s;
}

elf64_section *elf64_get_section_by_name(elf64_contents *contents, str *name) {
    mempool *scratch = new_mempool();
    elf64_section *target = NULL;
    iterator *it = llist_create_iterator(contents->sections, scratch);
    for_iterator(elf64_section, s, it) {
        if (str_equals(s->name, name)) {
            target = s;
            break;
        }
    }
    mempool_release(scratch);
    return target;
}

elf64_section *elf64_get_section_by_index(elf64_contents *contents, int index) {
    mempool *scratch = new_mempool();
    elf64_section *target = NULL;
    iterator *it = llist_create_iterator(contents->sections, scratch);
    for_iterator(elf64_section, s, it) {
        if (s->index == index) {
            target = s;
            break;
        }
    }
    mempool_release(scratch);
    return target;
}

elf64_section *elf64_get_section_by_type(elf64_contents *contents, int type) {
    mempool *scratch = new_mempool();
    elf64_section *target = NULL;
    iterator *it = llist_create_iterator(contents->sections, scratch);
    for_iterator(elf64_section, s, it) {
        if (s->header->type == type) {
            target = s;
            break;
        }
    }
    mempool_release(scratch);
    return target;
}


// -------------------------------------------

static inline size_t round_up(size_t value, size_t granularity) {
    return (((value + granularity - 1) / granularity) * granularity);
}

elf64_header *new_elf64_file_header(mempool *mp, bool executable, u64 entry_point) {
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

elf64_prog_header *new_elf64_prog_header(mempool *mp, int type, int flags, int align, u64 file_offset, u64 file_size, u64 mem_addr, u64 mem_size) {
    elf64_prog_header *h = mempool_alloc(mp, sizeof(elf64_prog_header), "elf64_prog_header");

    h->type = type;
    h->flags = flags;
    h->align = align;
    h->file_offset = file_offset;
    h->file_size = file_size;
    h->phys_address = mem_addr;
    h->virt_address = mem_addr;
    h->memory_size = mem_size;

    return h;
}

elf64_section_header *new_elf64_section_header(int type, char *name, u64 flags, u64 file_offset, u64 size, u64 item_size, u64 link, u64 info, u64 virt_address, bin *sections_names_table, mempool *mp) {
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

static bin *flatten_elf64_contents(elf64_contents *contents, mempool *mp) {
    mempool *scratch = new_mempool();
    bin *file_data = new_bin(mp);
    iterator *it;

    /*
        File structure is as follows
        ----------------------------------------
        [ elf file header      ]
        [ program header 0     ]  <-- skipped for .o files
        [ program header 1     ]
        [ program header ...   ]
        [ section 1   contents ]  <-- these are text, data, bss, etc contents
        [ section 2   contents ]
        [ section ... contents ]
        [ section 0   header   ]  <-- section headers
        [ section 1   header   ]
        [ section ... header   ]
    */

    bin_add_mem(file_data, contents->header, sizeof(elf64_header));

    // save curr offset and prog headers stuff
    contents->header->prog_headers_entry_size = sizeof(elf64_prog_header);
    contents->header->prog_headers_entries = llist_length(contents->prog_headers);
    contents->header->prog_headers_offset = llist_length(contents->prog_headers) == 0 ? 0 : bin_len(file_data);
    
    // all program headers first
    it = llist_create_iterator(contents->prog_headers, scratch);
    for_iterator(elf64_prog_header, h, it)
        bin_add_mem(file_data, h, sizeof(elf64_prog_header));
    

    // find or create a header names section
    elf64_section *header_names_section = NULL;
    if (contents->header->section_headers_strings_entry > 0) {
        header_names_section = llist_get(contents->sections, contents->header->section_headers_strings_entry);
    } else {
        contents->header->section_headers_strings_entry = llist_length(contents->sections);
        header_names_section = new_elf64_section(contents->mempool);
        header_names_section->header->type = SECTION_TYPE_STRTAB;
        header_names_section->name = new_str(contents->mempool, ".shstrtab");
        llist_add(contents->sections, header_names_section);
    }
    bin_clear(header_names_section->contents);
    bin_add_byte(header_names_section->contents, 0);


    // write all section contents
    it = llist_create_iterator(contents->sections, scratch);
    for_iterator(elf64_section, s, it) {
        s->header->name = bin_len(header_names_section->contents);
        bin_add_str(header_names_section->contents, s->name); // add name before calculating size
        s->header->file_offset = bin_len(file_data);
        s->header->size = (s->header->type == SECTION_TYPE_NOBITS) ? 0 : bin_len(s->contents);
        
        // contents of NOBITS sections should not be saved
        if (s->header->type == SECTION_TYPE_NOBITS)
            continue;

        bin_cat(file_data, s->contents);
    }

    // keep note where the section headers will be
    contents->header->section_headers_entry_size = sizeof(elf64_section_header);
    contents->header->section_headers_entries = llist_length(contents->sections);
    contents->header->section_headers_offset = llist_length(contents->sections) == 0 ? 0 : bin_len(file_data);

    // write all section headers
    it = llist_create_iterator(contents->sections, scratch);
    for_iterator(elf64_section, s, it) {
        bin_add_mem(file_data, s->header, sizeof(elf64_section_header));
    }


    // finally, rewrite header, to update the offsets of the various tables
    bin_seek(file_data, 0);
    bin_write_mem(file_data, contents->header, sizeof(elf64_header));


    // done
    mempool_release(scratch);
    return file_data;
}

bool elf64_contents_save(char *filename, elf64_contents *contents) {

    mempool *mp = new_mempool();
    bin *file_contents = flatten_elf64_contents(contents, mp);
    bool saved = bin_save_to_file(file_contents, new_str(mp, filename));
    mempool_release(mp);

    return saved;
}

void elf64_contents_print(elf64_contents *contents, FILE *stream) {
    mempool *mp = new_mempool();
    char *section_types[] = { "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NUM" };
    char *prog_types[] = { "NULL", "LOAD", "DYNAMIC", "INTERP", "NOTE", "SHLIB", "PHDR", "TLS" };

    fprintf(stream, "ELF64 Contents (magic = %c%c%c%c)\n", 
        contents->header->identity[0], contents->header->identity[1], contents->header->identity[2], contents->header->identity[3]);
    fprintf(stream, "  Type: %d (1=relocatable, 2=executable, 3=dynamic executable)\n", contents->header->file_type);
    
    if (llist_length(contents->prog_headers) > 0) {
        fprintf(stream, "  Program headers\n");
        fprintf(stream, "    Type         Offset    Virt Addr    Phys Addr  FileSize   MemSiz Flg Align\n");
        //              "    1234567890 12345678 123456789012 123456789012  12345678 12345678 XXX 12345678"
        iterator *prog_headers_iterator = llist_create_iterator(contents->prog_headers, mp);
        for_iterator(elf64_prog_header, h, prog_headers_iterator) {
            fprintf(stream, "    %-10s %08lx %012lx %012lx %08lx %08lx %c%c%c %ld\n",
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
    fprintf(stream, "    Num Name                 Type        Address   Offset     Size Flg Lk Inf Al\n");
    //              "    123 12345678901234567890 12345678 1234567890 12345678 12345678 123 12 123 12"
    iterator *sections_iterator = llist_create_iterator(contents->sections, mp);
    for_iterator(elf64_section, s, sections_iterator) {
        fprintf(stream, "    %3d %-20s %-8s %10ld %08lx %08lx %c%c%c %2d %3d %2ld\n",
            s->index,
            str_charptr(s->name),
            s->header->type >= 0 && s->header->type < (sizeof(section_types)/sizeof(section_types[0])) ? section_types[s->header->type] : "?",
            s->header->virt_address,
            s->header->file_offset,
            s->header->size,
            s->header->flags & SECTION_FLAGS_ALLOC ? 'A' : ' ',
            s->header->flags & SECTION_FLAGS_WRITE ? 'W' : ' ',
            s->header->flags & SECTION_FLAGS_EXECINSTR ? 'X' : ' ',
            s->header->info,
            s->header->link,
            s->header->address_alignment
        );
    }

    mempool_release(mp);
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
    char *obj_filename = "temp_unit_test_d06ffeae-d503-45ea-868e-9e36b73d1f69.o";

    // save and load an obj file.
    elf64_contents *contents = new_elf64_contents(mp);
    elf64_section *s;

    s = new_elf64_section(mp);
    s->name = new_str(mp, ".text");
    s->header->type = SECTION_TYPE_PROGBITS;
    bin_pad(s->contents, 'T', 64);
    llist_add(contents->sections, s);

    s = new_elf64_section(mp);
    s->name = new_str(mp, ".data");
    s->header->type = SECTION_TYPE_PROGBITS;
    bin_pad(s->contents, 'D', 64);
    llist_add(contents->sections, s);

    s = new_elf64_section(mp);
    s->name = new_str(mp, ".bss");
    s->header->type = SECTION_TYPE_NOBITS;
    bin_pad(s->contents, 'B', 64);
    llist_add(contents->sections, s);

    s = new_elf64_section(mp);
    s->name = new_str(mp, ".rodata");
    s->header->type = SECTION_TYPE_PROGBITS;
    bin_pad(s->contents, 'R', 64);
    llist_add(contents->sections, s);

    bool elf_obj_saved = elf64_contents_save(obj_filename, contents);
    assert(elf_obj_saved);

    // pause and check using `readelf`
    // assume time passes ... (which it always does)

    bin *file_data = new_bin_from_file(mp, new_str(mp, obj_filename));
    elf64_contents *loaded = new_elf64_contents_from_binary(mp, file_data);
    unlink(obj_filename);
    assert(loaded != NULL);
    assert(llist_length(loaded->sections) >= 4);

    s = elf64_get_section_by_name(contents, new_str(mp, ".text"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "TTTTTTTT", 8) == 0);

    s = elf64_get_section_by_name(contents, new_str(mp, ".data"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "DDDDDDDD", 8) == 0);

    // bss is not written, nor read
    s = elf64_get_section_by_name(contents, new_str(mp, ".rodata"));
    assert(s != NULL);
    assert(memcmp(bin_ptr_at(s->contents, 0), "RRRRRRRR", 8) == 0);
}
#endif // INCLUDE_UNIT_TESTS

