#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../../options.h"
#include "../../utils.h"
#include "elf_contents.h"
#include "elf_format.h"



/*
    we need to treat sections in a unitform way, in a loop.
    - elf header
    - program headers
    - contents
    - section headers
    prepare them all, then iterate.
*/

static void pad_file_to_size(FILE *f, long size);



static void write_elf32_file(elf_contents *prog, FILE *f) {

    // write main header
    elf32_header *header = malloc(sizeof(elf32_header));
    memset(header, 0, sizeof(elf32_header));
    memcpy(header->identity, "\177ELF", 4);
    header->identity[ELF_IDENTITY_CLASS] = prog->flags.is_64_bits ? ELF_CLASS_64 : ELF_CLASS_32;
    header->identity[ELF_IDENTITY_DATA] = ELF_DATA2_LSB;
    header->identity[ELF_IDENTITY_VERSION] = ELF_VERSION_CURRENT;
    header->identity[ELF_IDENTITY_OS_ABI] = ELF_OSABI_SYSV;
    if      (prog->flags.is_object_code)        header->file_type = ELF_TYPE_REL;
    else if (prog->flags.is_static_executable)  header->file_type = ELF_TYPE_EXEC;
    else if (prog->flags.is_dynamic_executable) header->file_type = ELF_TYPE_DYN;
    header->version = ELF_VERSION_CURRENT;
    header->entry_point = prog->code_entry_point;
    header->machine = prog->flags.is_64_bits ? ELF_MACHINE_X86_64 : ELF_MACHINE_386;
    header->elf_header_size = sizeof(elf32_header);
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf32_header), f);

    elf32_prog_header *empty_ph = malloc(sizeof(elf32_prog_header));
    elf32_prog_header *text_ph = malloc(sizeof(elf32_prog_header));
    elf32_prog_header *data_ph = malloc(sizeof(elf32_prog_header));
    elf32_prog_header *bss_ph = malloc(sizeof(elf32_prog_header));
    memset(empty_ph, 0, sizeof(elf32_prog_header));
    memset(text_ph, 0, sizeof(elf32_prog_header));
    memset(data_ph, 0, sizeof(elf32_prog_header));
    memset(bss_ph, 0, sizeof(elf32_prog_header));
    bool need_prog_headers = prog->flags.is_dynamic_executable || prog->flags.is_static_executable;

    if (need_prog_headers) {
        // write program headers
        header->prog_headers_offset = ftell(f);
        header->prog_headers_entry_size = sizeof(elf32_prog_header);
        header->prog_headers_entries = 3; // first, code, data
        if (prog->bss_size > 0)
            header->prog_headers_entries += 1; // bss is optional

        empty_ph->type = PROG_TYPE_LOAD;
        empty_ph->file_offset = 0;
        empty_ph->file_size = header->prog_headers_offset + header->prog_headers_entry_size * header->prog_headers_entries;
        empty_ph->memory_size = empty_ph->file_size;
        empty_ph->align = 0x1000;
        empty_ph->flags = PROG_FLAGS_READ;
        empty_ph->virt_address = prog->code_address - 0x1000;
        empty_ph->phys_address = prog->code_address - 0x1000;
        fwrite(empty_ph, 1, sizeof(elf32_prog_header), f);

        text_ph->type = PROG_TYPE_LOAD;
        text_ph->file_offset = 0; // we'll come back to fill this.
        text_ph->file_size = prog->code_size;
        text_ph->memory_size = prog->code_size;
        text_ph->align = 0x1000;
        text_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_EXECUTE;
        text_ph->virt_address = prog->code_address;
        text_ph->phys_address = prog->code_address;
        fwrite(text_ph, 1, sizeof(elf32_prog_header), f);

        data_ph->type = PROG_TYPE_LOAD;
        data_ph->file_offset = 0; // we'll come back to fill this
        data_ph->file_size = prog->data_size;
        data_ph->memory_size = prog->data_size;
        data_ph->align = 0x1000;
        data_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        data_ph->virt_address = prog->data_address;
        data_ph->phys_address = prog->data_address;
        fwrite(data_ph, 1, sizeof(elf32_prog_header), f);

        if (prog->bss_size > 0) {
            bss_ph->type = PROG_TYPE_LOAD;
            bss_ph->file_offset = 0; // we'll come back to fill this
            bss_ph->file_size = 0;
            bss_ph->memory_size = prog->bss_size;
            bss_ph->align = 1;
            bss_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
            bss_ph->virt_address = prog->bss_address;
            bss_ph->phys_address = prog->bss_address;
            fwrite(bss_ph, 1, sizeof(elf32_prog_header), f);
        }
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
    text_section->virt_address = prog->code_address;
    text_section->size = prog->code_size;
    text_section->name = strings_len;
    text_section->address_alignment = 16;
    strcat(strings + strings_len, ".text");
    strings_len += strlen(".text") + 1;
    header->entry_point = prog->code_entry_point;
    pad_file_to_size(f, round_up(ftell(f), text_ph->align));
    text_section->file_offset = ftell(f);
    fwrite(prog->code_contents, 1, prog->code_size, f);
    header->section_headers_entries++;

    data_section->type = SECTION_TYPE_PROGBITS;
    data_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    data_section->virt_address = prog->data_address;
    data_section->size = prog->data_size;
    data_section->name = strings_len;
    data_section->address_alignment = 4;
    strcat(strings + strings_len, ".data");
    strings_len += strlen(".data") + 1;
    pad_file_to_size(f, round_up(ftell(f), data_ph->align));
    data_section->file_offset = ftell(f);
    fwrite(prog->data_contents, 1, prog->data_size, f);
    header->section_headers_entries++;

    if (prog->bss_size > 0) {
        bss_section->type = SECTION_TYPE_NOBITS;
        bss_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
        bss_section->virt_address = prog->bss_address;
        bss_section->size = prog->bss_size;
        bss_section->name = strings_len;
        bss_section->address_alignment = 4;
        strcat(strings + strings_len, ".bss");
        strings_len += strlen(".bss") + 1;
        // remember, we don't write anything actually
        header->section_headers_entries++;
    }

    // strings table (contents)
    strings_section->type = SECTION_TYPE_STRTAB;
    strings_section->name = strings_len;
    strcat(strings + strings_len, ".shstrtab");
    strings_len += strlen(".shstrtab") + 1;
    strings_section->size = strings_len;
    strings_section->address_alignment = 1;
    strings_section->file_offset = ftell(f);
    fwrite(strings, 1, strings_len, f);
    header->section_headers_strings_entry = header->section_headers_entries;
    header->section_headers_entries++;

    // now we can write the section headers, with file offsets
    header->section_headers_offset = ftell(f);
    header->section_headers_entry_size = sizeof(elf32_section_header);
    fwrite(empty_section,   1, sizeof(elf32_section_header), f);
    fwrite(text_section,    1, sizeof(elf32_section_header), f);
    fwrite(data_section,    1, sizeof(elf32_section_header), f);
    if (prog->bss_size > 0)
        fwrite(bss_section,     1, sizeof(elf32_section_header), f);
    fwrite(strings_section, 1, sizeof(elf32_section_header), f);

    if (need_prog_headers) {
        empty_ph->file_offset = 0;
        text_ph->file_offset = text_section->file_offset;
        data_ph->file_offset = data_section->file_offset;

        // go back and write program headers, now with file offsets
        fseek(f, header->prog_headers_offset, SEEK_SET);
        fwrite(empty_ph, 1, sizeof(elf32_prog_header), f);
        fwrite(text_ph, 1, sizeof(elf32_prog_header), f);
        fwrite(data_ph, 1, sizeof(elf32_prog_header), f);
        if (prog->bss_size > 0)
            fwrite(bss_ph, 1, sizeof(elf32_prog_header), f);
    }

    // go back and write the whole header, now that we have header table offset
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf32_header), f);

    free(header);
    free(text_ph);
    free(data_ph);
    free(bss_ph);
    free(empty_section);
    free(text_section);
    free(data_section);
    free(bss_section);
    free(strings_section);
    free(strings);
}

static void write_elf64_file(elf_contents *prog, FILE *f) {

    // write main header
    elf64_header *header = malloc(sizeof(elf64_header));
    memset(header, 0, sizeof(elf64_header));
    memcpy(header->identity, "\177ELF", 4);
    header->identity[ELF_IDENTITY_CLASS] = prog->flags.is_64_bits ? ELF_CLASS_64 : ELF_CLASS_64;
    header->identity[ELF_IDENTITY_DATA] = ELF_DATA2_LSB;
    header->identity[ELF_IDENTITY_VERSION] = ELF_VERSION_CURRENT;
    header->identity[ELF_IDENTITY_OS_ABI] = ELF_OSABI_SYSV;
    if      (prog->flags.is_object_code)        header->file_type = ELF_TYPE_REL;
    else if (prog->flags.is_static_executable)  header->file_type = ELF_TYPE_EXEC;
    else if (prog->flags.is_dynamic_executable) header->file_type = ELF_TYPE_DYN;
    header->version = ELF_VERSION_CURRENT;
    header->entry_point = prog->code_entry_point;
    header->machine = prog->flags.is_64_bits ? ELF_MACHINE_X86_64 : ELF_MACHINE_386;
    header->elf_header_size = sizeof(elf64_header);
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf64_header), f);

    elf64_prog_header *text_ph = malloc(sizeof(elf64_prog_header));
    elf64_prog_header *data_ph = malloc(sizeof(elf64_prog_header));
    elf64_prog_header *bss_ph = malloc(sizeof(elf64_prog_header));
    memset(text_ph, 0, sizeof(elf64_prog_header));
    memset(data_ph, 0, sizeof(elf64_prog_header));
    memset(bss_ph, 0, sizeof(elf64_prog_header));
    bool need_prog_headers = prog->flags.is_dynamic_executable || prog->flags.is_static_executable;

    if (need_prog_headers) {
        // write program headers
        header->prog_headers_offset = ftell(f);
        header->prog_headers_entry_size = sizeof(elf64_prog_header);
        header->prog_headers_entries = 3;

        text_ph->type = PROG_TYPE_LOAD;
        text_ph->file_size = prog->code_size;
        text_ph->memory_size = prog->code_size;
        text_ph->align = 4;
        data_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_EXECUTE;
        text_ph->virt_address = prog->code_address;
        text_ph->phys_address = prog->code_address;
        fwrite(text_ph, 1, sizeof(elf64_prog_header), f);

        data_ph->type = PROG_TYPE_LOAD;
        data_ph->file_size = prog->data_size;
        data_ph->memory_size = prog->data_size;
        data_ph->align = 4;
        data_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        data_ph->virt_address = prog->data_address;
        data_ph->phys_address = prog->data_address;
        fwrite(data_ph, 1, sizeof(elf64_prog_header), f);

        bss_ph->type = PROG_TYPE_LOAD;
        bss_ph->file_size = 0;
        bss_ph->memory_size = prog->bss_size;
        bss_ph->align = 4;
        bss_ph->flags = PROG_FLAGS_READ | PROG_FLAGS_WRITE;
        bss_ph->virt_address = prog->bss_address;
        bss_ph->phys_address = prog->bss_address;
        fwrite(bss_ph, 1, sizeof(elf64_prog_header), f);
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
    text_section->virt_address = prog->code_address;
    text_section->size = prog->code_size;
    text_section->name = strings_len;
    strcat(strings + strings_len, ".text");
    strings_len += strlen(".text") + 1;
    header->entry_point = prog->code_entry_point;
    text_section->file_offset = ftell(f);
    fwrite(prog->code_contents, 1, prog->code_size, f);
    header->section_headers_entries++;

    data_section->type = SECTION_TYPE_PROGBITS;
    data_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    data_section->virt_address = prog->data_address;
    data_section->size = prog->data_size;
    data_section->file_offset = ftell(f);
    data_section->name = strings_len;
    strcat(strings + strings_len, ".data");
    strings_len += strlen(".data") + 1;
    fwrite(prog->data_contents, 1, prog->data_size, f);
    header->section_headers_entries++;

    bss_section->type = SECTION_TYPE_NOBITS;
    bss_section->flags = SECTION_FLAGS_ALLOC | SECTION_FLAGS_WRITE;
    bss_section->virt_address = prog->bss_address;
    bss_section->size = prog->bss_size;
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
        text_ph->file_offset = text_section->file_offset;
        data_ph->file_offset = data_section->file_offset;
        bss_ph->file_offset = bss_section->file_offset;

        // go back and write program headers, now with file offsets
        fseek(f, header->prog_headers_offset, SEEK_SET);
        fwrite(text_ph, 1, sizeof(elf64_prog_header), f);
        fwrite(data_ph, 1, sizeof(elf64_prog_header), f);
        fwrite(bss_ph, 1, sizeof(elf64_prog_header), f);
    }

    // go back and write the whole header, now that we have header table offsets
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf64_header), f);

    free(header);
    free(text_ph);
    free(data_ph);
    free(bss_ph);
    free(empty_section);
    free(text_section);
    free(data_section);
    free(bss_section);
    free(strings_section);
    free(strings);
}


bool write_elf_file(elf_contents *prog, char *filename) {
    FILE *f = fopen(filename, "w+");
    if (f == NULL) {
        printf("Error opening file %s for writing\n", filename);
        return false;
    }

    if (prog->flags.is_64_bits)
        write_elf64_file(prog, f);
    else
        write_elf32_file(prog, f);

    return true;
}

static void pad_file_to_size(FILE *f, long size) {
    char padding[256];
    memset(padding, 0, 256);

    long pos = ftell(f);
    long remaining = size - pos;

    while (remaining > 256) {
        fwrite(padding, 1, 256, f);
        remaining -= 256;
    }
    if (remaining > 0)
        fwrite(padding, 1, remaining, f);
}
