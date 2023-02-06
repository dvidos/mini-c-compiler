#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include "../options.h"
#include "binary.h"
#include "elf_format.h"


char *elf_file_type_names[] = { "NONE", "REL - object code", "EXEC - statically linked executable", "DYN - executable requiring dyn libraries", "CORE" };
char *elf_class_names[] = { "NONE", "32 bits", "64 bits" };
char *elf_data_encoding_names[] = { "NONE", "LSB first", "MSB first" };
char *elf_prog_type_names[] = { "NULL", "LOAD", "DYN", "INTER", "NOTE", "SHLIB", "PHDR", "TLS" };
char *elf_section_type_names[] = { "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NUM" };

#define ARR(arr, ndx)   (ndx > 0 && ndx < (sizeof(arr) / sizeof(arr[0]))) ? arr[ndx] : "???"

void printf16hex(void *address, int size);

void perform_elf_test_executable_32() {

}
void perform_elf_test_executable_64() {

}
void perform_elf_test_object_32() {

}
void perform_elf_test_object_64() {

}

bool read_elf32_file(FILE *f) {
    size_t bytes;
    int i;
    elf32_offset o;
    
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
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], ARR(elf_class_names, hdr->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], ARR(elf_data_encoding_names, hdr->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", hdr->file_type, ARR(elf_file_type_names, hdr->file_type));
    printf("  Machine    : %d\n", hdr->machine);
    printf("  Version    : %d\n", hdr->version);
    printf("  Entry point: 0x%x\n", hdr->entry_point);
    printf("  Flags      : 0x%x\n", hdr->flags);
    printf("  Header size: %d\n", hdr->elf_header_size);

    printf("  Program Headers - Num %u, Size %u, Offset %u\n", 
        hdr->prog_headers_entries, hdr->prog_headers_entry_size, hdr->prog_headers_offset);
    if (hdr->prog_headers_offset > 0) {
        printf("    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg\n");
        //          nn XXXXXX 123456789 123456789 0x00000000 0x00000000 123456789 1234 XXX
        elf32_prog_header *ph = malloc(sizeof(elf32_prog_header));
        for (o = hdr->prog_headers_offset, i = 0; i < hdr->prog_headers_entries; o += hdr->prog_headers_entry_size) {
            fseek(f, o, SEEK_SET);
            bytes = fread(ph, 1, sizeof(elf32_prog_header), f);
            if (bytes < sizeof(elf32_prog_header))
                return false;
            printf("    %2d %-6s %9d %9d 0x%08x 0x%08x %9d %4d %c%c%c\n",
                i, ARR(elf_prog_type_names, ph->type),
                ph->file_offset, ph->file_size,
                ph->virt_address, ph->phys_address, ph->memory_size,
                ph->align,
                ph->flags & PROG_FLAGS_READ ? 'R' : '.',
                ph->flags & PROG_FLAGS_WRITE ? 'W' : '.',
                ph->flags & PROG_FLAGS_EXECUTE ? 'X' : '.'
            );
            i++;
        }
        free(ph);
    }

    printf("  Section Headers - Num %u, Size %u, Offset %u  (sh str ndx %d)\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset, hdr->section_headers_strings_entry);
    if (hdr->section_headers_offset > 0) {
        elf32_section_header *sh = malloc(sizeof(elf32_section_header));
        char *symbols = NULL;
        // let's grab symbols first
        if (hdr->section_headers_strings_entry > 0) {
            fseek(f, hdr->section_headers_offset + hdr->section_headers_entry_size * hdr->section_headers_strings_entry, SEEK_SET);
            bytes = fread(sh, 1, sizeof(elf32_section_header), f);
            if (bytes < sizeof(elf32_section_header)) return false;
            fseek(f, sh->file_offset, SEEK_SET);
            symbols = malloc(sh->size);
            bytes = fread(symbols, 1, sh->size, f);
            if (bytes < sh->size) return false;
            // printf16hex(symbols, sh->size);
            //   00 2e 73 79 6d 74 61 62   00 2e 73 74 72 74 61 62  ..symtab ..strtab
            //   00 2e 73 68 73 74 72 74   61 62 00 2e 72 65 6c 2e  ..shstrt ab..rel.
        }
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
        for (o = hdr->section_headers_offset, i = 0; i < hdr->section_headers_entries; o += hdr->section_headers_entry_size) {
            fseek(f, o, SEEK_SET);
            bytes = fread(sh, 1, sizeof(elf32_section_header), f);
            if (bytes < sizeof(elf32_section_header))
                return false;
            printf("    %2d %-20s %-8s %9d %9d   %4d %c%c%c\n",
                i, 
                symbols == NULL ? "(unknown)" : (sh->name == 0 ? "(none)" : symbols + sh->name),
                ARR(elf_section_type_names, sh->type),
                sh->file_offset,
                sh->size,
                sh->address_alignment,
                sh->flags & SECTION_FLAGS_WRITE ? 'W' : '.',
                sh->flags & SECTION_FLAGS_ALLOC ? 'A' : '.',
                sh->flags & SECTION_FLAGS_EXECINSTR ? 'X' : '.'
            );
            i++;
        }
        if (symbols != NULL) free(symbols);
        free(sh);
    }
    return true;
}
bool read_elf64_file(FILE *f) {
    size_t bytes;
    int i;
    elf64_offset o;

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
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], ARR(elf_class_names, hdr->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], ARR(elf_data_encoding_names, hdr->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", hdr->file_type, ARR(elf_file_type_names, hdr->file_type));
    printf("  Machine    : %d\n", hdr->machine);
    printf("  Version    : %d\n", hdr->version);
    printf("  Entry point: 0x%lx\n", hdr->entry_point);
    printf("  Flags      : 0x%x\n", hdr->flags);
    printf("  Header size: %d\n", hdr->elf_header_size);
    printf("  Program Headers - Num %u, Size %u, Offset %lu\n", 
        hdr->prog_headers_entries, hdr->prog_headers_entry_size, hdr->prog_headers_offset);
    if (hdr->prog_headers_offset > 0) {
        printf("    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg\n");
        //          nn XXXXXX 123456789 123456789 0x00000000 0x00000000 123456789 1234 XXX
        elf64_prog_header *ph = malloc(sizeof(elf64_prog_header));
        for (o = hdr->prog_headers_offset, i = 0; i < hdr->prog_headers_entries; o += hdr->prog_headers_entry_size) {
            fseek(f, o, SEEK_SET);
            bytes = fread(ph, 1, sizeof(elf64_prog_header), f);
            if (bytes < sizeof(elf64_prog_header))
                return false;
            printf("    %2d %-6s %9ld %9ld 0x%08lx 0x%08lx %9ld %4ld %c%c%c\n",
                i, ARR(elf_prog_type_names, ph->type),
                ph->file_offset, ph->file_size,
                ph->virt_address, ph->phys_address, ph->memory_size,
                ph->align,
                ph->flags & PROG_FLAGS_READ ? 'R' : '.',
                ph->flags & PROG_FLAGS_WRITE ? 'W' : '.',
                ph->flags & PROG_FLAGS_EXECUTE ? 'X' : '.'
            );
            i++;
        }
        free(ph);
    }

    printf("  Section Headers - Num %u, Size %u, Offset %lu  (sh str ndx %d)\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset, hdr->section_headers_strings_entry);
    if (hdr->section_headers_offset > 0) {
        elf64_section_header *sh = malloc(sizeof(elf64_section_header));
        char *symbols = NULL;
        // let's grab symbols first
        if (hdr->section_headers_strings_entry > 0) {
            fseek(f, hdr->section_headers_offset + hdr->section_headers_entry_size * hdr->section_headers_strings_entry, SEEK_SET);
            bytes = fread(sh, 1, sizeof(elf64_section_header), f);
            if (bytes < sizeof(elf64_section_header)) return false;
            fseek(f, sh->file_offset, SEEK_SET);
            symbols = malloc(sh->size);
            bytes = fread(symbols, 1, sh->size, f);
            if (bytes < sh->size) return false;
            // printf16hex(symbols, sh->size);
            //   00 2e 73 79 6d 74 61 62   00 2e 73 74 72 74 61 62  ..symtab ..strtab
            //   00 2e 73 68 73 74 72 74   61 62 00 2e 72 65 6c 2e  ..shstrt ab..rel.
        }
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
        for (o = hdr->section_headers_offset, i = 0; i < hdr->section_headers_entries; o += hdr->section_headers_entry_size) {
            fseek(f, o, SEEK_SET);
            bytes = fread(sh, 1, sizeof(elf64_section_header), f);
            if (bytes < sizeof(elf64_section_header))
                return false;
            printf("    %2d %-20s %-8s %9ld %9ld   %4ld %c%c%c\n",
                i, 
                symbols == NULL ? "(unknown)" : (sh->name == 0 ? "(none)" : symbols + sh->name),
                ARR(elf_section_type_names, sh->type),
                sh->file_offset,
                sh->size,
                sh->address_alignment,
                sh->flags & SECTION_FLAGS_WRITE ? 'W' : '.',
                sh->flags & SECTION_FLAGS_ALLOC ? 'A' : '.',
                sh->flags & SECTION_FLAGS_EXECINSTR ? 'X' : '.'
            );
            i++;
        }
        if (symbols != NULL) free(symbols);
        free(sh);
    }

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

    printf("ELF file information: [%s]\n", filename);
    if (buff[ELF_IDENTITY_CLASS] == ELF_CLASS_32) {
        read_elf32_file(f);
    }
    if (buff[ELF_IDENTITY_CLASS] == ELF_CLASS_64) {
        read_elf64_file(f);
    }
    printf("\n");
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
    try_reading_elf_file("mcc");
    try_reading_elf_file("./docs/bin/tiny-obj32");
    try_reading_elf_file("./docs/bin/tiny-obj64");
    try_reading_elf_file("./docs/bin/tiny-dyn32");
    try_reading_elf_file("./docs/bin/tiny-dyn64");
    try_reading_elf_file("./docs/bin/tiny-stat32");
    try_reading_elf_file("./docs/bin/tiny-stat64");
}




void printf16hex(void *address, int size) {
    char *p = address;
    while (size > 0) {
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
        p += 16;
        size -= 16;
    }
}


