#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "elf_format.h"

char *elf_file_type_names[] = { "NONE", "REL - object code", "EXEC - statically linked executable", "DYN - executable requiring dyn libraries", "CORE" };
char *elf_class_names[] = { "NONE", "32 bits", "64 bits" };
char *elf_data_encoding_names[] = { "NONE", "LSB first", "MSB first" };
char *elf_prog_type_names[] = { "NULL", "LOAD", "DYN", "INTER", "NOTE", "SHLIB", "PHDR", "TLS" };
char *elf_section_type_names[] = { "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NUM", "13", "INIT_ARR", "FINI_ARR", "PREINIT_ARR", "GROUP", "SYMTAB_SHNDX" };
#define NM(arr, ndx)   (ndx >= 0 && ndx < (sizeof(arr) / sizeof(arr[0]))) ? arr[ndx] : "???"



static bool read_elf32_file(FILE *f) {
    size_t bytes;
    int i;
    elf32_offset offs;
    
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
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], NM(elf_class_names, hdr->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], NM(elf_data_encoding_names, hdr->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", hdr->file_type, NM(elf_file_type_names, hdr->file_type));
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
        elf32_prog_header *prog = malloc(sizeof(elf32_prog_header));
        for (offs = hdr->prog_headers_offset, i = 0; i < hdr->prog_headers_entries; offs += hdr->prog_headers_entry_size) {
            fseek(f, offs, SEEK_SET);
            bytes = fread(prog, 1, sizeof(elf32_prog_header), f);
            if (bytes < sizeof(elf32_prog_header))
                return false;
            printf("    %2d %-6s %9d %9d 0x%08x 0x%08x %9d %4d %c%c%c\n",
                i, NM(elf_prog_type_names, prog->type),
                prog->file_offset, prog->file_size,
                prog->virt_address, prog->phys_address, prog->memory_size,
                prog->align,
                prog->flags & PROG_FLAGS_READ ? 'R' : '.',
                prog->flags & PROG_FLAGS_WRITE ? 'W' : '.',
                prog->flags & PROG_FLAGS_EXECUTE ? 'X' : '.'
            );
            i++;
        }
        free(prog);
    }

    printf("  Section Headers - Num %u, Size %u, Offset %u  (section str ndx %d)\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset, hdr->section_headers_strings_entry);
    if (hdr->section_headers_offset > 0) {
        elf32_section_header *section = malloc(sizeof(elf32_section_header));
        char *symbols = NULL;
        // let's grab symbols first
        if (hdr->section_headers_strings_entry > 0) {
            fseek(f, hdr->section_headers_offset + hdr->section_headers_entry_size * hdr->section_headers_strings_entry, SEEK_SET);
            bytes = fread(section, 1, sizeof(elf32_section_header), f);
            if (bytes < sizeof(elf32_section_header)) return false;
            fseek(f, section->file_offset, SEEK_SET);
            symbols = malloc(section->size);
            bytes = fread(symbols, 1, section->size, f);
            if (bytes < section->size) return false;
            // printf16hex(symbols, section->size);
            //   00 2e 73 79 6d 74 61 62   00 2e 73 74 72 74 61 62  ..symtab ..strtab
            //   00 2e 73 68 73 74 72 74   61 62 00 2e 72 65 6c 2e  ..shstrt ab..rel.
        }
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
        for (offs = hdr->section_headers_offset, i = 0; i < hdr->section_headers_entries; offs += hdr->section_headers_entry_size) {
            fseek(f, offs, SEEK_SET);
            bytes = fread(section, 1, sizeof(elf32_section_header), f);
            if (bytes < sizeof(elf32_section_header))
                return false;
            printf("    %2d %-20s %-8s %9d %9d   %4d %c%c%c\n",
                i, 
                symbols == NULL ? "(unknown)" : (section->name == 0 ? "(none)" : symbols + section->name),
                NM(elf_section_type_names, section->type),
                section->file_offset,
                section->size,
                section->address_alignment,
                section->flags & SECTION_FLAGS_WRITE ? 'W' : '.',
                section->flags & SECTION_FLAGS_ALLOC ? 'A' : '.',
                section->flags & SECTION_FLAGS_EXECINSTR ? 'X' : '.'
            );
            i++;
        }
        if (symbols != NULL) free(symbols);
        free(section);
    }
    return true;
}

static bool read_elf64_file(FILE *f) {
    size_t bytes;
    int i;
    elf64_offset offs;

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
    printf("  Class      : %d (%s)\n", hdr->identity[ELF_IDENTITY_CLASS], NM(elf_class_names, hdr->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", hdr->identity[ELF_IDENTITY_DATA], NM(elf_data_encoding_names, hdr->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", hdr->file_type, NM(elf_file_type_names, hdr->file_type));
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
        elf64_prog_header *prog = malloc(sizeof(elf64_prog_header));
        for (offs = hdr->prog_headers_offset, i = 0; i < hdr->prog_headers_entries; offs += hdr->prog_headers_entry_size) {
            fseek(f, offs, SEEK_SET);
            bytes = fread(prog, 1, sizeof(elf64_prog_header), f);
            if (bytes < sizeof(elf64_prog_header))
                return false;
            printf("    %2d %-6s %9ld %9ld 0x%08lx 0x%08lx %9ld %4ld %c%c%c\n",
                i, NM(elf_prog_type_names, prog->type),
                prog->file_offset, prog->file_size,
                prog->virt_address, prog->phys_address, prog->memory_size,
                prog->align,
                prog->flags & PROG_FLAGS_READ ? 'R' : '.',
                prog->flags & PROG_FLAGS_WRITE ? 'W' : '.',
                prog->flags & PROG_FLAGS_EXECUTE ? 'X' : '.'
            );
            i++;
        }
        free(prog);
    }

    printf("  Section Headers - Num %u, Size %u, Offset %lu  (section str ndx %d)\n", 
        hdr->section_headers_entries, hdr->section_headers_entry_size, hdr->section_headers_offset, hdr->section_headers_strings_entry);
    if (hdr->section_headers_offset > 0) {
        elf64_section_header *section = malloc(sizeof(elf64_section_header));
        char *symbols = NULL;
        // let's grab symbols first
        if (hdr->section_headers_strings_entry > 0) {
            fseek(f, hdr->section_headers_offset + hdr->section_headers_entry_size * hdr->section_headers_strings_entry, SEEK_SET);
            bytes = fread(section, 1, sizeof(elf64_section_header), f);
            if (bytes < sizeof(elf64_section_header)) return false;
            fseek(f, section->file_offset, SEEK_SET);
            symbols = malloc(section->size);
            bytes = fread(symbols, 1, section->size, f);
            if (bytes < section->size) return false;
            // printf16hex(symbols, section->size);
            //   00 2e 73 79 6d 74 61 62   00 2e 73 74 72 74 61 62  ..symtab ..strtab
            //   00 2e 73 68 73 74 72 74   61 62 00 2e 72 65 6c 2e  ..shstrt ab..rel.
        }
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
        for (offs = hdr->section_headers_offset, i = 0; i < hdr->section_headers_entries; offs += hdr->section_headers_entry_size) {
            fseek(f, offs, SEEK_SET);
            bytes = fread(section, 1, sizeof(elf64_section_header), f);
            if (bytes < sizeof(elf64_section_header))
                return false;
            printf("    %2d %-20s %-8s %9ld %9ld   %4ld %c%c%c\n",
                i, 
                symbols == NULL ? "(unknown)" : (section->name == 0 ? "(none)" : symbols + section->name),
                NM(elf_section_type_names, section->type),
                section->file_offset,
                section->size,
                section->address_alignment,
                section->flags & SECTION_FLAGS_WRITE ? 'W' : '.',
                section->flags & SECTION_FLAGS_ALLOC ? 'A' : '.',
                section->flags & SECTION_FLAGS_EXECINSTR ? 'X' : '.'
            );
            i++;
        }
        if (symbols != NULL) free(symbols);
        free(section);
    }

    return true;
}

void read_elf_file(char *filename) {
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



