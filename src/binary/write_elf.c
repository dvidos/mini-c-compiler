#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../options.h"
#include "object_code.h"
#include "elf_format.h"
#include "read_elf.h"


static void write_elf32_file(object_code *code, FILE *f) {

    // write main header
    elf32_header *header = malloc(sizeof(elf32_header));
    memset(header, 0, sizeof(elf32_header));
    memcpy(header->identity, "\177ELF", 4);
    header->identity[ELF_IDENTITY_CLASS] = code->flags.is_64_bits ? ELF_CLASS_64 : ELF_CLASS_32;
    header->identity[ELF_IDENTITY_DATA] = ELF_DATA2_LSB;
    header->identity[ELF_IDENTITY_VERSION] = ELF_VERSION_CURRENT;
    header->identity[ELF_IDENTITY_OS_ABI] = ELF_OSABI_SYSV;
    if      (code->flags.is_object_code)        header->file_type = ELF_TYPE_REL;
    else if (code->flags.is_static_executable)  header->file_type = ELF_TYPE_EXEC;
    else if (code->flags.is_dynamic_executable) header->file_type = ELF_TYPE_DYN;
    header->version = ELF_VERSION_CURRENT;
    header->entry_point = code->code_entry_point;
    header->machine = code->flags.is_64_bits ? ELF_MACHINE_X86_64 : ELF_MACHINE_386;
    header->elf_header_size = sizeof(elf32_header);
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf32_header), f);

    elf32_prog_header *prog_text = malloc(sizeof(elf32_prog_header));
    elf32_prog_header *prog_data = malloc(sizeof(elf32_prog_header));
    elf32_prog_header *prog_bss = malloc(sizeof(elf32_prog_header));
    memset(prog_text, 0, sizeof(elf32_prog_header));
    memset(prog_data, 0, sizeof(elf32_prog_header));
    memset(prog_bss, 0, sizeof(elf32_prog_header));
    bool need_prog_headers = code->flags.is_dynamic_executable || code->flags.is_static_executable;

    if (need_prog_headers) {
        // write program headers
        header->prog_headers_offset = ftell(f);
        header->prog_headers_entry_size = sizeof(elf32_prog_header);
        header->prog_headers_entries = 3;

        prog_text->type = PROG_TYPE_LOAD;
        prog_text->file_size = code->code_size;
        prog_text->memory_size = code->code_size;
        prog_text->align = 4;
        prog_data->flags = PROG_FLAGS_READ | PROG_FLAGS_EXECUTE;
        prog_text->virt_address = code->code_address;
        prog_text->phys_address = code->code_address;
        fwrite(prog_text, 1, sizeof(elf32_prog_header), f);

        prog_data->type = PROG_TYPE_LOAD;
        prog_data->file_size = code->init_data_size;
        prog_data->memory_size = code->init_data_size;
        prog_data->align = 4;
        prog_data->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        prog_data->virt_address = code->init_data_address;
        prog_data->phys_address = code->init_data_address;
        fwrite(prog_data, 1, sizeof(elf32_prog_header), f);

        prog_bss->type = PROG_TYPE_LOAD;
        prog_bss->file_size = 0;
        prog_bss->memory_size = code->zero_data_size;
        prog_bss->align = 4;
        prog_bss->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        prog_bss->virt_address = code->zero_data_address;
        prog_bss->phys_address = code->zero_data_address;
        fwrite(prog_bss, 1, sizeof(elf32_prog_header), f);
    }
    
    // prepare to write the section contents
    elf32_section_header *empty_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *text_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *data_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *bss_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *strings_section = malloc(sizeof(elf32_section_header));
    memset(empty_section, 0, sizeof(elf32_section_header));
    memset(text_section, 0, sizeof(elf32_section_header));
    memset(data_section, 0, sizeof(elf32_section_header));
    memset(bss_section, 0, sizeof(elf32_section_header));
    memset(strings_section, 0, sizeof(elf32_section_header));

    char *strings = malloc(512);
    memset(strings, 0, 512);
    int strings_len = 1; // skip first terminator

    // first section header is null, we don't store anything
    empty_section->type = SECTION_TYPE_NULL;
    header->section_headers_entries++;

    // write the sections contents
    text_section->type = SECTION_TYPE_PROGBITS;
    text_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_EXECINSTR;
    text_section->virt_address = code->code_address;
    text_section->size = code->code_size;
    text_section->name = strings_len;
    strcat(strings + strings_len, ".text");
    strings_len += strlen(".text") + 1;
    header->entry_point = code->code_entry_point;
    text_section->file_offset = ftell(f);
    fwrite(code->code_contents, 1, code->code_size, f);
    header->section_headers_entries++;

    data_section->type = SECTION_TYPE_PROGBITS;
    data_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    data_section->virt_address = code->init_data_address;
    data_section->size = code->init_data_size;
    data_section->file_offset = ftell(f);
    data_section->name = strings_len;
    strcat(strings + strings_len, ".data");
    strings_len += strlen(".data") + 1;
    fwrite(code->init_data_contents, 1, code->init_data_size, f);
    header->section_headers_entries++;

    bss_section->type = SECTION_TYPE_NOBITS;
    bss_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    bss_section->virt_address = code->zero_data_address;
    bss_section->size = code->zero_data_size;
    bss_section->name = strings_len;
    strcat(strings + strings_len, ".bss");
    strings_len += strlen(".bss") + 1;
    // fwrite(NULL, 0, 0, f); // nothing actually
    header->section_headers_entries++;

    // strings table (contents)
    strings_section->type = SECTION_TYPE_STRTAB;
    strings_section->name = strings_len;
    strings_section->file_offset = ftell(f);
    strcat(strings + strings_len, ".shstrtab");
    strings_len += strlen(".shstrtab") + 1;
    strings_section->size = strings_len;
    fwrite(strings, 1, strings_len, f);
    header->section_headers_strings_entry = header->section_headers_entries;
    header->section_headers_entries++;

    // now we can write the section headers, with file offsets
    header->section_headers_offset = ftell(f);
    header->section_headers_entry_size = sizeof(elf32_section_header);
    fwrite(empty_section,   1, sizeof(elf32_section_header), f);
    fwrite(text_section,    1, sizeof(elf32_section_header), f);
    fwrite(data_section,    1, sizeof(elf32_section_header), f);
    fwrite(bss_section,     1, sizeof(elf32_section_header), f);
    fwrite(strings_section, 1, sizeof(elf32_section_header), f);

    if (need_prog_headers) {
        prog_text->file_offset = text_section->file_offset;
        prog_data->file_offset = data_section->file_offset;
        prog_bss->file_offset = bss_section->file_offset;

        // go back and write program headers, now with file offsets
        fseek(f, header->prog_headers_offset, SEEK_SET);
        fwrite(prog_text, 1, sizeof(elf32_prog_header), f);
        fwrite(prog_data, 1, sizeof(elf32_prog_header), f);
        fwrite(prog_bss, 1, sizeof(elf32_prog_header), f);
    }

    // go back and write the whole header, now that we have header table offsets
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf32_header), f);

    free(header);
    free(prog_text);
    free(prog_data);
    free(prog_bss);
    free(empty_section);
    free(text_section);
    free(data_section);
    free(bss_section);
    free(strings_section);
    free(strings);
}

static void write_elf64_file(object_code *code, FILE *f) {

    // write main header
    elf64_header *header = malloc(sizeof(elf64_header));
    memset(header, 0, sizeof(elf64_header));
    memcpy(header->identity, "\177ELF", 4);
    header->identity[ELF_IDENTITY_CLASS] = code->flags.is_64_bits ? ELF_CLASS_64 : ELF_CLASS_64;
    header->identity[ELF_IDENTITY_DATA] = ELF_DATA2_LSB;
    header->identity[ELF_IDENTITY_VERSION] = ELF_VERSION_CURRENT;
    header->identity[ELF_IDENTITY_OS_ABI] = ELF_OSABI_SYSV;
    if      (code->flags.is_object_code)        header->file_type = ELF_TYPE_REL;
    else if (code->flags.is_static_executable)  header->file_type = ELF_TYPE_EXEC;
    else if (code->flags.is_dynamic_executable) header->file_type = ELF_TYPE_DYN;
    header->version = ELF_VERSION_CURRENT;
    header->entry_point = code->code_entry_point;
    header->machine = code->flags.is_64_bits ? ELF_MACHINE_X86_64 : ELF_MACHINE_386;
    header->elf_header_size = sizeof(elf64_header);
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf64_header), f);

    elf64_prog_header *prog_text = malloc(sizeof(elf64_prog_header));
    elf64_prog_header *prog_data = malloc(sizeof(elf64_prog_header));
    elf64_prog_header *prog_bss = malloc(sizeof(elf64_prog_header));
    memset(prog_text, 0, sizeof(elf64_prog_header));
    memset(prog_data, 0, sizeof(elf64_prog_header));
    memset(prog_bss, 0, sizeof(elf64_prog_header));
    bool need_prog_headers = code->flags.is_dynamic_executable || code->flags.is_static_executable;

    if (need_prog_headers) {
        // write program headers
        header->prog_headers_offset = ftell(f);
        header->prog_headers_entry_size = sizeof(elf64_prog_header);
        header->prog_headers_entries = 3;

        prog_text->type = PROG_TYPE_LOAD;
        prog_text->file_size = code->code_size;
        prog_text->memory_size = code->code_size;
        prog_text->align = 4;
        prog_data->flags = PROG_FLAGS_READ | PROG_FLAGS_EXECUTE;
        prog_text->virt_address = code->code_address;
        prog_text->phys_address = code->code_address;
        fwrite(prog_text, 1, sizeof(elf64_prog_header), f);

        prog_data->type = PROG_TYPE_LOAD;
        prog_data->file_size = code->init_data_size;
        prog_data->memory_size = code->init_data_size;
        prog_data->align = 4;
        prog_data->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        prog_data->virt_address = code->init_data_address;
        prog_data->phys_address = code->init_data_address;
        fwrite(prog_data, 1, sizeof(elf64_prog_header), f);

        prog_bss->type = PROG_TYPE_LOAD;
        prog_bss->file_size = 0;
        prog_bss->memory_size = code->zero_data_size;
        prog_bss->align = 4;
        prog_bss->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        prog_bss->virt_address = code->zero_data_address;
        prog_bss->phys_address = code->zero_data_address;
        fwrite(prog_bss, 1, sizeof(elf64_prog_header), f);
    }
    
    // prepare to write the section contents
    elf64_section_header *empty_section = malloc(sizeof(elf64_section_header));
    elf64_section_header *text_section = malloc(sizeof(elf64_section_header));
    elf64_section_header *data_section = malloc(sizeof(elf64_section_header));
    elf64_section_header *bss_section = malloc(sizeof(elf64_section_header));
    elf64_section_header *strings_section = malloc(sizeof(elf64_section_header));
    memset(empty_section, 0, sizeof(elf64_section_header));
    memset(text_section, 0, sizeof(elf64_section_header));
    memset(data_section, 0, sizeof(elf64_section_header));
    memset(bss_section, 0, sizeof(elf64_section_header));
    memset(strings_section, 0, sizeof(elf64_section_header));

    char *strings = malloc(512);
    memset(strings, 0, 512);
    int strings_len = 1; // skip first terminator

    // first section header is null, we don't store anything
    empty_section->type = SECTION_TYPE_NULL;
    header->section_headers_entries++;

    // write the sections contents
    text_section->type = SECTION_TYPE_PROGBITS;
    text_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_EXECINSTR;
    text_section->virt_address = code->code_address;
    text_section->size = code->code_size;
    text_section->name = strings_len;
    strcat(strings + strings_len, ".text");
    strings_len += strlen(".text") + 1;
    header->entry_point = code->code_entry_point;
    text_section->file_offset = ftell(f);
    fwrite(code->code_contents, 1, code->code_size, f);
    header->section_headers_entries++;

    data_section->type = SECTION_TYPE_PROGBITS;
    data_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    data_section->virt_address = code->init_data_address;
    data_section->size = code->init_data_size;
    data_section->file_offset = ftell(f);
    data_section->name = strings_len;
    strcat(strings + strings_len, ".data");
    strings_len += strlen(".data") + 1;
    fwrite(code->init_data_contents, 1, code->init_data_size, f);
    header->section_headers_entries++;

    bss_section->type = SECTION_TYPE_NOBITS;
    bss_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    bss_section->virt_address = code->zero_data_address;
    bss_section->size = code->zero_data_size;
    bss_section->name = strings_len;
    strcat(strings + strings_len, ".bss");
    strings_len += strlen(".bss") + 1;
    // fwrite(NULL, 0, 0, f); // nothing actually
    header->section_headers_entries++;

    // strings table (contents)
    strings_section->type = SECTION_TYPE_STRTAB;
    strings_section->name = strings_len;
    strings_section->file_offset = ftell(f);
    strcat(strings + strings_len, ".shstrtab");
    strings_len += strlen(".shstrtab") + 1;
    strings_section->size = strings_len;
    fwrite(strings, 1, strings_len, f);
    header->section_headers_strings_entry = header->section_headers_entries;
    header->section_headers_entries++;

    // now we can write the section headers, with file offsets
    header->section_headers_offset = ftell(f);
    header->section_headers_entry_size = sizeof(elf64_section_header);
    fwrite(empty_section,   1, sizeof(elf64_section_header), f);
    fwrite(text_section,    1, sizeof(elf64_section_header), f);
    fwrite(data_section,    1, sizeof(elf64_section_header), f);
    fwrite(bss_section,     1, sizeof(elf64_section_header), f);
    fwrite(strings_section, 1, sizeof(elf64_section_header), f);

    if (need_prog_headers) {
        prog_text->file_offset = text_section->file_offset;
        prog_data->file_offset = data_section->file_offset;
        prog_bss->file_offset = bss_section->file_offset;

        // go back and write program headers, now with file offsets
        fseek(f, header->prog_headers_offset, SEEK_SET);
        fwrite(prog_text, 1, sizeof(elf64_prog_header), f);
        fwrite(prog_data, 1, sizeof(elf64_prog_header), f);
        fwrite(prog_bss, 1, sizeof(elf64_prog_header), f);
    }

    // go back and write the whole header, now that we have header table offsets
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf64_header), f);

    free(header);
    free(prog_text);
    free(prog_data);
    free(prog_bss);
    free(empty_section);
    free(text_section);
    free(data_section);
    free(bss_section);
    free(strings_section);
    free(strings);
}


bool write_elf_file(object_code *code, char *filename) {
    FILE *f = fopen(filename, "w+");
    if (f == NULL) {
        printf("Error opening file %s for writing\n", filename);
        return false;
    }

    if (code->flags.is_64_bits)
        write_elf64_file(code, f);
    else
        write_elf32_file(code, f);

    fclose(f);
}

