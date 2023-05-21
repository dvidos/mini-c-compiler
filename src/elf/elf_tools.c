#include <unistd.h>
#include <string.h>
#include "elf_format.h"
#include "elf_tools.h"

#define ALIGN_SIZE  0x1000

// -------------------------------------------

static elf_section *create_elf_section(mempool *mp) {
    elf_section *s = mempool_alloc(mp, sizeof(elf_section), "elf_section");
    s->contents = new_binary(mp);
    s->mem_address = 0;
    return s;
}

elf_contents2 *new_elf_contents2(mempool *mp) {
    elf_contents2 *ptr = mempool_alloc(mp, sizeof(elf_contents2), "elf_contents2");
    ptr->text      = create_elf_section(mp);
    ptr->data      = create_elf_section(mp);
    ptr->bss       = create_elf_section(mp);
    ptr->rodata    = create_elf_section(mp);
    ptr->rela_text = create_elf_section(mp);
    ptr->symtab    = create_elf_section(mp);
    ptr->strtab    = create_elf_section(mp);
    ptr->comment   = create_elf_section(mp);
    return ptr;
}

// -------------------------------------------

static inline size_t round_up(size_t value, size_t granularity) {
    return (((value + granularity - 1) / granularity) * granularity);
}

static elf64_header *create_file_header64(mempool *mp, bool executable, u64 entry_point, u64 prog_hdr_count, u64 prog_hdr_offset, u64 section_hdr_count, u64 section_hdr_offset, u64 section_names_strtab_index) {
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

    h->prog_headers_entries = prog_hdr_count;
    h->prog_headers_entry_size = sizeof(elf64_prog_header);
    h->prog_headers_offset = prog_hdr_count > 0 ? prog_hdr_offset : 0;

    h->section_headers_entries = section_hdr_count;
    h->section_headers_entry_size = sizeof(elf64_section_header);
    h->section_headers_offset = section_hdr_offset;
    h->section_headers_strings_entry = section_names_strtab_index;

    return h;
}

static elf64_prog_header *create_program_header64(mempool *mp, int type, int flags, int align, u64 file_offset, u64 file_size, u64 mem_addr, u64 mem_size) {
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

static elf64_section_header *create_section_header64(int type, char *name, u64 flags, u64 file_offset, u64 size, u64 item_size, u64 link, u64 info, u64 virt_address, binary *sections_names_table, mempool *mp) {
    elf64_section_header *h = mempool_alloc(mp, sizeof(elf64_section_header), "elf64_section_header");

    u64 name_offset = binary_length(sections_names_table);
    binary_add_mem(sections_names_table, name, strlen(name) + 1);

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

static binary *prepare_elf64(mempool *mp, elf_contents2 *contents, bool executable, u64 entry_point) {
    /*
        File structure is as follows

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
    binary *result = new_binary(mp);
    binary *section_names_table = new_binary(mp);
    
    u64 program_headers_offset = 0;
    u64 program_headers_count = 0;
    u64 section_headers_offset = 0;
    u64 section_headers_count = 0;
    u64 section_names_table_index = 0;
    u64 file_loading_info_length = 0;

    // file header placeholder
    binary_add_zeros(result, sizeof(elf64_header)); 

    if (executable) {
        // we shall create 5 loading headers: headers, text, data, bss, rodata
        program_headers_offset = binary_length(result);
        program_headers_count = 5;

        binary_add_zeros(result, sizeof(elf64_prog_header) * program_headers_count);
        
        // since we are loading the file headers table, pad this to a page size
        binary_pad(result, 0, round_up(binary_length(result), ALIGN_SIZE));
        file_loading_info_length = binary_length(result);
    }

    // do two things for each section:
    // - prepare header, noting current file offset, append name in names_table
    // - add content, to advance file offset

    /*

Sect#    Name          Notes
    0    <null>
    1    .text
    2    .data
    3    .bss
    4    .rodata
    5    .rela_text    link to 6 for str table, info to 1 for relevant code 
    6    .symtab       symbols, link to 7 for strings, info must be "One greater than the symbol table index of the last local symbol (binding STB_LOCAL)"
    7    .strtab       names of symbols
    8    .comment      
    9    .shstrtab     names of sections
    */

    // section 0
    elf64_section_header *empty_section_header = create_section_header64(
        0, "", 0, 
        binary_length(result), 0, 0, 0, 0, 0,
        section_names_table, mp);
    // no contents, this is an empty section
    
    // section 1
    elf64_section_header *text_header = create_section_header64(
        SECTION_TYPE_PROGBITS, ".text", SECTION_FLAGS_ALLOC | SECTION_FLAGS_EXECINSTR, 
        binary_length(result), binary_length(contents->text->contents), 0, 0, 0, 0,
        section_names_table, mp);
    binary_cat(result, contents->text->contents);

    elf64_section_header *data_header = create_section_header64(
        SECTION_TYPE_PROGBITS, ".data", SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE, 
        binary_length(result), binary_length(contents->data->contents), 0, 0, 0, 0,
        section_names_table, mp);
    binary_cat(result, contents->data->contents);

    elf64_section_header *bss_header = create_section_header64(
        SECTION_TYPE_NOBITS, ".bss", SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE, 
        binary_length(result), binary_length(contents->bss->contents), 0, 0, 0, 0,
        section_names_table, mp);
    // binary_cat(file_buffer, contents->rodata);  no storage for BSS segment

    elf64_section_header *rodata_header = create_section_header64(
        SECTION_TYPE_PROGBITS, ".rodata", SECTION_FLAGS_ALLOC, 
        binary_length(result), binary_length(contents->rodata->contents), 0, 0, 0, 0, 
        section_names_table, mp);
    binary_cat(result, contents->rodata->contents);

    elf64_section_header *rela_text_header = create_section_header64(
        SECTION_TYPE_RELA, ".rela.text", 0, 
        binary_length(result), binary_length(contents->rela_text->contents), sizeof(elf64_rela), 6, 1, 0,  
        section_names_table, mp);
    binary_cat(result, contents->rela_text->contents);

    elf64_section_header *symtab_header = create_section_header64(
        SECTION_TYPE_SYMTAB, ".symtab", 0, 
        binary_length(result), binary_length(contents->symtab->contents), sizeof(elf64_sym), 7, 1, 0, 
        section_names_table, mp);
    binary_cat(result, contents->symtab->contents);

    elf64_section_header *strtab_header = create_section_header64(
        SECTION_TYPE_STRTAB, ".strtab", 0, 
        binary_length(result), binary_length(contents->strtab->contents), 0, 0, 0, 0,
        section_names_table, mp);
    binary_cat(result, contents->strtab->contents);

    elf64_section_header *comment_header = create_section_header64(
        SECTION_TYPE_PROGBITS, ".comment", 0, 
        binary_length(result), binary_length(contents->comment->contents), 0, 0, 0, 0,
        section_names_table, mp);
    binary_cat(result, contents->comment->contents);

    // we grab the length of the names table before adding the ".shstrtab" entry, hence the +10.
    elf64_section_header *shstrtab_header = create_section_header64(
        SECTION_TYPE_STRTAB, ".shstrtab", 0, 
        binary_length(result), binary_length(section_names_table) + 10, 0, 0, 0, 0,
        section_names_table, mp);
    binary_cat(result, section_names_table);

    section_headers_offset = binary_length(result);
    section_headers_count = 10;
    section_names_table_index = 9;

    // save all section headers
    binary_add_mem(result, empty_section_header, sizeof(elf64_section_header));
    binary_add_mem(result, text_header,          sizeof(elf64_section_header));
    binary_add_mem(result, data_header,          sizeof(elf64_section_header));
    binary_add_mem(result, rodata_header,        sizeof(elf64_section_header));
    binary_add_mem(result, bss_header,           sizeof(elf64_section_header));
    binary_add_mem(result, rela_text_header,     sizeof(elf64_section_header));
    binary_add_mem(result, symtab_header,        sizeof(elf64_section_header));
    binary_add_mem(result, strtab_header,        sizeof(elf64_section_header));
    binary_add_mem(result, comment_header,       sizeof(elf64_section_header));
    binary_add_mem(result, shstrtab_header,      sizeof(elf64_section_header));

    // now that we have offsets and sizes, update program headers
    if (executable) {
        u64 load_headers_mem_address = contents->text->mem_address - round_up(file_loading_info_length, 0x1000);
        elf64_prog_header *load_headers_pheader = create_program_header64(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ, 0x1000,
            0,                        file_loading_info_length, 
            load_headers_mem_address, file_loading_info_length);
        
        elf64_prog_header *text_pheader = create_program_header64(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_EXECUTE, 0x1000,
            text_header->file_offset,    binary_length(contents->text->contents), 
            contents->text->mem_address, binary_length(contents->text->contents));
        
        elf64_prog_header *data_pheader = create_program_header64(mp, 
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_WRITE, 0x1000,
            data_header->file_offset,    binary_length(contents->data->contents), 
            contents->data->mem_address, binary_length(contents->data->contents));
        
        elf64_prog_header *bss_pheader = create_program_header64(mp,
            PROG_TYPE_LOAD, PROG_FLAGS_READ | PROG_FLAGS_WRITE, 0x1000,
            bss_header->file_offset,    0,
            contents->bss->mem_address, binary_length(contents->bss->contents));
        
        elf64_prog_header *rodata_pheader = create_program_header64(mp,
            PROG_TYPE_LOAD, PROG_FLAGS_READ, 0x1000,
            rodata_header->file_offset,    binary_length(contents->rodata->contents), 
            contents->rodata->mem_address, binary_length(contents->rodata->contents));
        
        binary_seek(result, program_headers_offset);
        binary_write_mem(result, load_headers_pheader, sizeof(elf64_prog_header));
        binary_write_mem(result, text_pheader,         sizeof(elf64_prog_header));
        binary_write_mem(result, data_pheader,         sizeof(elf64_prog_header));
        binary_write_mem(result, bss_pheader,          sizeof(elf64_prog_header));
        binary_write_mem(result, rodata_pheader,       sizeof(elf64_prog_header));
    }

    // now that we have all offsets, update file header
    elf64_header *header = create_file_header64(mp, 
        executable, entry_point,
        program_headers_count, program_headers_offset, 
        section_headers_count, section_headers_offset, section_names_table_index);
    binary_seek(result, 0);
    binary_write_mem(result, header, sizeof(elf64_header));

    return result;
}


bool save_elf64_executable(char *filename, elf_contents2 *contents, u64 entry_point) {

    // all loadable sections MUST be page aligned (from linker), otherwise they cannot load
    if (contents->text->mem_address & (ALIGN_SIZE - 1)
        || contents->data->mem_address & (ALIGN_SIZE - 1)
        || contents->bss->mem_address & (ALIGN_SIZE - 1)
        || contents->rodata->mem_address & (ALIGN_SIZE - 1))
        return false;
    
    if ((binary_length(contents->text->contents) & (ALIGN_SIZE - 1))
        || (binary_length(contents->data->contents) & (ALIGN_SIZE - 1))
        || (binary_length(contents->bss->contents) & (ALIGN_SIZE - 1))
        || (binary_length(contents->rodata->contents) & (ALIGN_SIZE - 1)))
        return false;

    mempool *mp = new_mempool();
    binary *file_contents = prepare_elf64(mp, contents, true, entry_point);
    bool saved = binary_save_to_file(file_contents, new_str(mp, filename));
    mempool_release(mp);

    return saved;
}

bool save_elf64_obj_file(char *filename, elf_contents2 *contents) {
    // we don't care about program headers or alignment

    mempool *mp = new_mempool();
    binary *file_contents = prepare_elf64(mp, contents, false, 0);
    bool saved = binary_save_to_file(file_contents, new_str(mp, filename));
    mempool_release(mp);

    return saved;
}

// -----------------------------------------------------------------------------

static bool parse_elf64_obj_header(binary *file_data, u64 *sections_offset, u64 *sections_count, u64 *section_names_table_index) {
    elf64_header the_header;
    elf64_header *h = &the_header;

    binary_seek(file_data, 0);
    binary_read_mem(file_data, h, sizeof(elf64_header));

    if (memcmp(h->identity, "\177ELF", 4) != 0)
        return false;
    
    if (h->identity[ELF_IDENTITY_CLASS] != ELF_CLASS_64)
        return false;
    
    if (h->file_type != ELF_TYPE_REL)
        return false;

    *sections_offset = h->section_headers_offset;
    *sections_count = h->section_headers_entries;
    *section_names_table_index = h->section_headers_strings_entry;
    return true;
}

static void parse_elf64_section(int section_no, binary *file_data, u64 sections_offset, binary *names_table, 
    str *out_name, binary *out_content)
{
    mempool *scratch = new_mempool();
    char two_bytes[2];

    // first read the section header
    elf64_section_header section;
    binary_seek(file_data, sections_offset + section_no * sizeof(elf64_section_header));
    binary_read_mem(file_data, &section, sizeof(elf64_section_header));

    // extract the name from the names table (if supported)
    if (out_name != NULL && names_table != NULL) {
        str_clear(out_name);
        binary_seek(names_table, section.name);
        two_bytes[0] = binary_read_byte(names_table);
        two_bytes[1] = 0;
        while (two_bytes[0] != 0) {
            str_cats(out_name, two_bytes);
            two_bytes[0] = binary_read_byte(names_table);
        }
    }

    // extract the section content
    binary_clear(out_content);
    binary *extracted = binary_get_slice(file_data, section.file_offset, section.size, scratch);
    binary_cat(out_content, extracted);

    mempool_release(scratch);
}

elf_contents2 *load_elf64_obj_file(mempool *mp, char *filename) {
    mempool *scratch = new_mempool();

    binary *file_data = new_binary_from_file(scratch, new_str(mp, filename));
    if (file_data == NULL)
        return NULL;

    u64 sections_offset;
    u64 sections_count;
    u64 section_names_table_index;

    if (!parse_elf64_obj_header(file_data, &sections_offset, &sections_count, &section_names_table_index))
        return NULL;
    
    // we need to load the section names
    binary *names_table = new_binary(scratch);
    if (section_names_table_index > 0)
        parse_elf64_section(section_names_table_index, file_data, sections_offset, NULL, NULL, names_table);

    elf_contents2 *result = new_elf_contents2(mp);
    str *sect_name = new_str(scratch, NULL);
    binary *sect_content = new_binary(scratch);

    // read each section, load it into memory
    for (int sect_no = 0; sect_no < sections_count; sect_no++) {
        parse_elf64_section(sect_no, file_data, sections_offset, names_table, sect_name, sect_content);
        
        elf_section *target = NULL;
        if      (str_cmps(sect_name, ".text")      == 0) target = result->text;
        else if (str_cmps(sect_name, ".data")      == 0) target = result->data;
        else if (str_cmps(sect_name, ".bss")       == 0) target = result->bss;
        else if (str_cmps(sect_name, ".rodata")    == 0) target = result->rodata;
        else if (str_cmps(sect_name, ".rela.text") == 0) target = result->rela_text;
        else if (str_cmps(sect_name, ".symtab")    == 0) target = result->symtab;
        else if (str_cmps(sect_name, ".strtab")    == 0) target = result->strtab;
        else if (str_cmps(sect_name, ".comment")   == 0) target = result->comment;
        else continue;

        binary_cat(target->contents, sect_content);
    }
    
    mempool_release(scratch);
    return result;
}

// -------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
static void elf_unit_test_obj_file(mempool *mp);
static void elf_unit_test_executable_file(mempool *mp);

void elf_unit_tests() {
    mempool *mp = new_mempool();

    elf_contents2 *ptr = new_elf_contents2(mp);
    assert(ptr != NULL);
    assert(ptr->text != NULL);
    assert(ptr->text->contents != NULL);

    elf_unit_test_obj_file(mp);
    elf_unit_test_executable_file(mp);

    mempool_release(mp);
}

static void elf_unit_test_obj_file(mempool *mp) {
    char buffer[64];
    char *obj_filename = "temp_unit_test_d06ffeae-d503-45ea-868e-9e36b73d1f69.o";

    // save and load an obj file.
    elf_contents2 *saved = new_elf_contents2(mp);

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "text contents");
    binary_add_mem(saved->text->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "data contents");
    binary_add_mem(saved->data->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "bss contents");
    binary_add_mem(saved->bss->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "rodata contents");
    binary_add_mem(saved->rodata->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "rela_text contents");
    binary_add_mem(saved->rela_text->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "symtab contents");
    binary_add_mem(saved->symtab->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "strtab contents");
    binary_add_mem(saved->strtab->contents, buffer, sizeof(buffer));

    memset(buffer, '-', sizeof(buffer));
    strcpy(buffer, "comment contents");
    binary_add_mem(saved->comment->contents, buffer, sizeof(buffer));

    bool elf_obj_saved = save_elf64_obj_file(obj_filename, saved);
    assert(elf_obj_saved);

    // pause and check using `readelf`
    // assume time passes ... (which it does)

    elf_contents2 *loaded = load_elf64_obj_file(mp, obj_filename);
    assert(loaded != NULL);

    unlink(obj_filename);

    binary_read_mem(loaded->text->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "text contents") == 0);

    binary_read_mem(loaded->data->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "data contents") == 0);

    binary_read_mem(loaded->rodata->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "rodata contents") == 0);

    binary_read_mem(loaded->rela_text->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "rela_text contents") == 0);

    binary_read_mem(loaded->symtab->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "symtab contents") == 0);

    binary_read_mem(loaded->strtab->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "strtab contents") == 0);

    binary_read_mem(loaded->comment->contents, buffer, sizeof(buffer));
    assert(strcmp(buffer, "comment contents") == 0);
}

static void elf_unit_test_executable_file(mempool *mp) {
    // generate the smallest executable,
    // read it using readelf in console,
    // load it here and compare chunks

    char buffer[64];
    char *exec_filename = "temp_unit_test_d06ffeae-d503-45ea-868e-9e36b73d1f69";

    // save and load an obj file.
    elf_contents2 *saved = new_elf_contents2(mp);

    strcpy(buffer, "text contents");
    binary_add_mem(saved->text->contents, buffer, sizeof(buffer));
    binary_pad(saved->text->contents, 0, 4096);

    strcpy(buffer, "data contents");
    binary_add_mem(saved->data->contents, buffer, sizeof(buffer));
    binary_pad(saved->data->contents, 0, 4096);

    strcpy(buffer, "bss contents");
    binary_add_mem(saved->bss->contents, buffer, sizeof(buffer));
    binary_pad(saved->bss->contents, 0, 4096);
    
    strcpy(buffer, "rodata contents");
    binary_add_mem(saved->rodata->contents, buffer, sizeof(buffer));
    binary_pad(saved->rodata->contents, 0, 4096);

    strcpy(buffer, "symtab contents");
    binary_add_mem(saved->symtab->contents, buffer, sizeof(buffer));

    strcpy(buffer, "strtab contents");
    binary_add_mem(saved->strtab->contents, buffer, sizeof(buffer));

    strcpy(buffer, "comment contents");
    binary_add_mem(saved->comment->contents, buffer, sizeof(buffer));

    bool elf_executable_saved = save_elf64_executable(exec_filename, saved, 0x12345678);
    assert(elf_executable_saved);

    // now read it and compare chnks
    binary *elf_data = new_binary_from_file(mp, new_str(mp, exec_filename));

    // we can verify specific bytes, sizes etc.
    binary_seek(elf_data, 0);
    assert(binary_read_byte(elf_data) == 0x7F);
    assert(binary_read_byte(elf_data) == 'E');
    assert(binary_read_byte(elf_data) == 'L');
    assert(binary_read_byte(elf_data) == 'F');

    // insert other tests here.
}
#endif // INCLUDE_UNIT_TESTS

