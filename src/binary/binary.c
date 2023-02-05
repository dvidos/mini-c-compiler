#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../options.h"
#include "binary.h"
#include "elf_format.h"

void printf16hex(void *address);
char *elf_file_type_names[] = { "NONE", "REL - object code", "EXEC - statically linked executable", "DYN - executable requiring dyn libraries", "CORE" };
char *elf_class_names[] = { "NONE", "32 bits", "64 bits" };
char *elf_data_encoding_names[] = { "NONE", "LSB first", "MSB first" };





void perform_elf_test_executable_32() {

}
void perform_elf_test_executable_64() {

}
void perform_elf_test_object_32() {

}
void perform_elf_test_object_64() {

}

bool read_elf32_file(FILE *f, char *filename) {
    size_t bytes;
    printf("ELF information - %s\n", filename);
    elf32_header *hdr = malloc(sizeof(elf32_header));
    fseek(f, 0, SEEK_CUR);
    bytes = fread(hdr, 1, sizeof(elf32_header), f);
    if (bytes < sizeof(elf32_header)) {
        return false;
    }
    printf("  Identity   : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        hdr->identity[0], hdr->identity[1], hdr->identity[2], hdr->identity[3], 
        hdr->identity[4], hdr->identity[5], hdr->identity[6], hdr->identity[7], 
        hdr->identity[8], hdr->identity[9], hdr->identity[10], hdr->identity[11], 
        hdr->identity[12], hdr->identity[13], hdr->identity[14], hdr->identity[15]
    );
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], elf_class_names[hdr->identity[ELF_IDENTITY_CLASS]]);
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], elf_data_encoding_names[hdr->identity[ELF_IDENTITY_DATA]]);
    printf("  File type  : %d (%s)\n", hdr->file_type, elf_file_type_names[hdr->file_type]);
    printf("  Machine    : %d\n", hdr->machine);
    printf("  Version    : %d\n", hdr->version);
    printf("  Entry point: 0x%x\n", hdr->entry_point);
    printf("  Flags      : 0x%x\n", hdr->flags);
    printf("  Header size: %d\n", hdr->elf_header_size);
    printf("  Program Headers - Num %u, Size %u, Offset %u\n", 
        hdr->prog_headers_entries, hdr->prog_headers_entry_size, hdr->prog_headers_offset);
    printf("  Section Headers - Num %u, Size %u, Offset %u\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset);
    printf("  Section header string table index: %d\n", hdr->section_headers_strings_entry);

    // printf("Header hex dump:\n");
    // printf16hex(((char *)hdr));
    // printf16hex(((char *)hdr)+16);
    // printf16hex(((char *)hdr)+32);

    return true;
}
bool read_elf64_file(FILE *f, char *filename) {
    size_t bytes;
    printf("ELF information - %s\n", filename);
    elf64_header *hdr = malloc(sizeof(elf64_header));
    fseek(f, 0, SEEK_CUR);
    bytes = fread(hdr, 1, sizeof(elf64_header), f);
    if (bytes < sizeof(elf64_header)) {
        return false;
    }
    printf("  Identity   : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        hdr->identity[0], hdr->identity[1], hdr->identity[2], hdr->identity[3], 
        hdr->identity[4], hdr->identity[5], hdr->identity[6], hdr->identity[7], 
        hdr->identity[8], hdr->identity[9], hdr->identity[10], hdr->identity[11], 
        hdr->identity[12], hdr->identity[13], hdr->identity[14], hdr->identity[15]
    );
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], elf_class_names[hdr->identity[ELF_IDENTITY_CLASS]]);
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], elf_data_encoding_names[hdr->identity[ELF_IDENTITY_DATA]]);
    printf("  File type  : %d (%s)\n", hdr->file_type, elf_file_type_names[hdr->file_type]);
    printf("  Machine    : %d\n", hdr->machine);
    printf("  Version    : %d\n", hdr->version);
    printf("  Entry point: 0x%lx\n", hdr->entry_point);
    printf("  Flags      : 0x%x\n", hdr->flags);
    printf("  Header size: %d\n", hdr->elf_header_size);
    printf("  Program Headers - Num %u, Size %u, Offset %lu\n", 
        hdr->prog_headers_entries, hdr->prog_headers_entry_size, hdr->prog_headers_offset);
    printf("  Section Headers - Num %u, Size %u, Offset %lu\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset);
    printf("  Section header string table index: %d\n", hdr->section_headers_strings_entry);

    // printf("Header hex dump:\n");
    // printf16hex(((char *)hdr));
    // printf16hex(((char *)hdr)+16);
    // printf16hex(((char *)hdr)+64);

    return true;
}

void try_reading_elf_file(char *filename) {
    char buff[32];
    size_t bytes;

    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        printf("Error opening file %s\n", filename);
        return;
    }
    bytes = fread(buff, 1, 16, f);
    if (bytes < 16) {
        printf("Error reading from file %s\n", filename);
        return;
    }
    fseek(f, 0, SEEK_SET);

    if (memcmp(buff, "\177ELF", 4) != 0) {
        printf("File %s is not an ELF file\n", filename);
        return;
    }
    // printf("  %s header: %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
    //     filename,
    //     buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6], buff[7], 
    //     buff[8], buff[9], buff[10], buff[11], buff[12], buff[13], buff[14], buff[15]
    // );
    if (buff[ELF_IDENTITY_CLASS] != ELF_CLASS_32 && buff[ELF_IDENTITY_CLASS] != ELF_CLASS_64) {
        printf("File %s is neither a 32 bits, nor a 64 bits (class = 0x%x)\n", filename, buff[ELF_IDENTITY_CLASS]);
        return;
    }
    if (buff[ELF_IDENTITY_CLASS] == ELF_CLASS_32) {
        read_elf32_file(f, filename);
    }
    if (buff[ELF_IDENTITY_CLASS] == ELF_CLASS_64) {
        read_elf64_file(f, filename);
    }
    fclose(f);
}

void try_writing_elf_file(char *filename, char bits, bool executable) {

    object_code *c = malloc(sizeof(object_code));

    elf32_header *header = malloc(sizeof(elf32_header));
    memset(header, 0, sizeof(elf32_header));
    memcpy(header->identity, "\177ELF", 4);
    header->identity[4] = ELF_CLASS_32;   // <-- note
    header->identity[5] = ELF_DATA2_LSB;
    header->identity[6] = ELF_VERSION_CURRENT; // the rest of identity is zero
    header->file_type = ELF_TYPE_EXEC;  // or ELF_TYPE_REL for objects
    header->machine = ELF_MACHINE_386;  // or X86_64 for 64 bits
    header->version = ELF_VERSION_CURRENT;
    header->entry_point = 0;
    header->flags = 0;
    header->elf_header_size = sizeof(elf32_header);
    

    // it seems both sections and programs point to segments.
    // any byte in the file belongs to one section at most
    // program headers usually follow the main header
    // program headers are to tell the LOADER how to load and run a process
    // section headers are to tell the LINKER what the sections are for linking into the executable
    // for executable files, program headers are mandatory
    // for object (relocatable) files, section headers are mandatory



    // sections (describe what is in the table)
    // 0 - NULL
    // 1 .text
    // 2 .data
    // 3 .bss
    // 4 shstrtab -- setion header strings table

    // programs?

    elf32_section_header *text_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *data_section = malloc(sizeof(elf32_section_header));
    elf32_section_header *strings_section = malloc(sizeof(elf32_section_header));


    
    


    
    // char *filename = "a.out"; // safe bet!
    FILE *f = fopen(filename, "w");
    fseek(f, 0, SEEK_SET);
    fwrite(header, 1, sizeof(elf32_header), f);
    long where = ftell(f);
    fclose(f);
}

void perform_elf_test() {
    printf("sizeof u8 %d, u16 %d, u32 %d, u64 %d\n", (int)sizeof(u8), (int)sizeof(u16), (int)sizeof(u32), (int)sizeof(u64));
    printf("sizeof s8 %d, s16 %d, s32 %d, s64 %d\n", (int)sizeof(s8), (int)sizeof(s16), (int)sizeof(s32), (int)sizeof(s64));
    try_reading_elf_file("mcc");
    try_reading_elf_file("./docs/bin/tiny-obj32");
    try_reading_elf_file("./docs/bin/tiny-obj64");
    try_reading_elf_file("./docs/bin/tiny-dyn32");
    try_reading_elf_file("./docs/bin/tiny-dyn64");
    try_reading_elf_file("./docs/bin/tiny-stat32");
    try_reading_elf_file("./docs/bin/tiny-stat64");
}




void printf16hex(void *address) {
    char *p = address;
    printf("    %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
        p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
        (p[0] > 32  && p[0]  < 127) ? p[0] : '.',
        (p[1] > 32  && p[1]  < 127) ? p[1] : '.',
        (p[2] > 32  && p[2]  < 127) ? p[2] : '.',
        (p[3] > 32  && p[3]  < 127) ? p[3] : '.',
        (p[4] > 32  && p[4]  < 127) ? p[4] : '.',
        (p[5] > 32  && p[5]  < 127) ? p[5] : '.',
        (p[6] > 32  && p[6]  < 127) ? p[6] : '.',
        (p[7] > 32  && p[7]  < 127) ? p[7] : '.',
        (p[8] > 32  && p[8]  < 127) ? p[8] : '.',
        (p[9] > 32  && p[9]  < 127) ? p[9] : '.',
        (p[10] > 32 && p[10] < 127) ? p[10] : '.',
        (p[11] > 32 && p[11] < 127) ? p[11] : '.',
        (p[12] > 32 && p[12] < 127) ? p[12] : '.',
        (p[13] > 32 && p[13] < 127) ? p[13] : '.',
        (p[14] > 32 && p[14] < 127) ? p[14] : '.',
        (p[15] > 32 && p[15] < 127) ? p[15] : '.'
    );
}


