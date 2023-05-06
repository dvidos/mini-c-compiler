#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "../../utils/buffer.h"
#include "../../utils/list.h"
#include "elf_format.h"
#include "elf_contents.h"

char *elf_file_type_names[] = { "NONE", "REL - object code", "EXEC - statically linked executable", "DYN - executable requiring dyn libraries", "CORE" };
char *elf_class_names[] = { "NONE", "32 bits", "64 bits" };
char *elf_data_encoding_names[] = { "NONE", "LSB first", "MSB first" };
char *elf_prog_type_names[] = { "NULL", "LOAD", "DYN", "INTER", "NOTE", "SHLIB", "PHDR", "TLS" };
char *elf_section_type_names[] = { "NULL", "PROGBITS", "SYMTAB", "STRTAB", "RELA", "HASH", "DYNAMIC", "NOTE", "NOBITS", "REL", "SHLIB", "DYNSYM", "NUM", "13", "INIT_ARR", "FINI_ARR", "PREINIT_ARR", "GROUP", "SYMTAB_SHNDX" };
char *elf_sym_bind_names[] = { "LOCAL", "GLOBAL", "WEAK" };
char *elf_sym_type_names[] = { "NOTYPE", "OBJECT", "FUNC", "SECTION", "FILE", "COMMON", "TLS" };
#define NM(arr, ndx)   (ndx >= 0 && ndx < (sizeof(arr) / sizeof(arr[0]))) ? arr[ndx] : "(no name available)"



static bool read_elf32_file(FILE *f, elf_contents *contents) {
    size_t bytes;
    int i;
    elf32_offset offs;
    
    elf32_header *header = malloc(sizeof(elf32_header));
    fseek(f, 0, SEEK_CUR);
    bytes = fread(header, 1, sizeof(elf32_header), f);
    if (bytes < sizeof(elf32_header)) {
        return false;
    }
    printf("  Identity   : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        header->identity[0], header->identity[1], header->identity[2], header->identity[3], 
        header->identity[4], header->identity[5], header->identity[6], header->identity[7], 
        header->identity[8], header->identity[9], header->identity[10], header->identity[11], 
        header->identity[12], header->identity[13], header->identity[14], header->identity[15]
    );
    printf("  Class      : %d (%s)\n", header->identity[ELF_IDENTITY_CLASS], NM(elf_class_names, header->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", header->identity[ELF_IDENTITY_DATA], NM(elf_data_encoding_names, header->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", header->file_type, NM(elf_file_type_names, header->file_type));
    printf("  Machine    : %d\n", header->machine);
    printf("  Version    : %d\n", header->version);
    printf("  Entry point: 0x%x\n", header->entry_point);
    printf("  Flags      : 0x%x\n", header->flags);
    printf("  Header size: %d\n", header->elf_header_size);

    printf("  Program Headers - Num %u, Size %u, Offset %u\n", 
        header->prog_headers_entries, header->prog_headers_entry_size, header->prog_headers_offset);
    if (header->prog_headers_offset > 0) {
        printf("    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg\n");
        //          nn XXXXXX 123456789 123456789 0x00000000 0x00000000 123456789 1234 XXX
        elf32_prog_header *prog = malloc(sizeof(elf32_prog_header));
        for (offs = header->prog_headers_offset, i = 0; i < header->prog_headers_entries; offs += header->prog_headers_entry_size) {
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
        header->section_headers_entries, header->section_headers_entry_size, header->section_headers_offset, header->section_headers_strings_entry);
    if (header->section_headers_offset > 0) {
        elf32_section_header *section = malloc(sizeof(elf32_section_header));
        char *symbols = NULL;
        // let's grab symbols first
        if (header->section_headers_strings_entry > 0) {
            fseek(f, header->section_headers_offset + header->section_headers_entry_size * header->section_headers_strings_entry, SEEK_SET);
            bytes = fread(section, 1, sizeof(elf32_section_header), f);
            if (bytes < sizeof(elf32_section_header)) return false;
            fseek(f, section->file_offset, SEEK_SET);
            symbols = malloc(section->size);
            bytes = fread(symbols, 1, section->size, f);
            if (bytes < section->size) return false;
            // print_16_hex(symbols, section->size);
            //   00 2e 73 79 6d 74 61 62   00 2e 73 74 72 74 61 62  ..symtab ..strtab
            //   00 2e 73 68 73 74 72 74   61 62 00 2e 72 65 6c 2e  ..shstrt ab..rel.
        }
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
        for (offs = header->section_headers_offset, i = 0; i < header->section_headers_entries; offs += header->section_headers_entry_size) {
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

            if (section->type == SECTION_TYPE_SYMTAB) {
                // let's try to see what this has
                fseek(f, section->file_offset, SEEK_SET);
                buffer *b = new_buffer();
                b->from_file(b, f, section->size);

                // in theory we should have a lot of symbols.
                int num_syms = b->length / sizeof(elf32_sym);
                for (int j = 0; j < num_syms; j++) {
                    elf32_sym *sym = (elf32_sym *)&b->buffer[j * sizeof(elf32_sym)];
                    char *sym_name = symbols == NULL ? "(unknown)" : (sym->st_name == 0 ? "(none)" : symbols + sym->st_name);
                }
            }
        }
        if (symbols != NULL) free(symbols);
        free(section);
    }



    return true;
}


// ------------------------------------------------------------------------------


static void elf64_print_file_header(elf64_header *header) {
    printf("  Identity   : %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
        header->identity[0], header->identity[1], header->identity[2], header->identity[3], 
        header->identity[4], header->identity[5], header->identity[6], header->identity[7], 
        header->identity[8], header->identity[9], header->identity[10], header->identity[11], 
        header->identity[12], header->identity[13], header->identity[14], header->identity[15]
    );
    printf("  Class      : %d (%s)\n", header->identity[ELF_IDENTITY_CLASS], NM(elf_class_names, header->identity[ELF_IDENTITY_CLASS]));
    printf("  Encoding   : %d (%s)\n", header->identity[ELF_IDENTITY_DATA], NM(elf_data_encoding_names, header->identity[ELF_IDENTITY_DATA]));
    printf("  File type  : %d (%s)\n", header->file_type, NM(elf_file_type_names, header->file_type));
    printf("  Machine    : %d\n", header->machine);
    printf("  Version    : %d\n", header->version);
    printf("  Entry point: 0x%lx\n", header->entry_point);
    printf("  Flags      : 0x%x\n", header->flags);
    printf("  Header size: %d\n", header->elf_header_size);
    printf("  Program Headers - Num %u, Size %u, Offset %lu\n", header->prog_headers_entries, header->prog_headers_entry_size, header->prog_headers_offset);
    printf("  Section Headers - Num %u, Size %u, Offset %lu  (section str ndx %d)\n", header->section_headers_entries, header->section_headers_entry_size, header->section_headers_offset, header->section_headers_strings_entry);
}

static void elf64_print_prog_header(elf64_prog_header *prog, int index) {
    if (prog == NULL) {
        printf("    P# Type    File pos   File sz     V.Addr     P.Addr    Mem Sz Algn Flg\n");
        //          nn 123456 123456789 123456789 0x12345678 0x12345678 123456789 1234 xxx
    } else {
        printf("    %2d %-6s %9ld %9ld 0x%08lx 0x%08lx %9ld %4ld %c%c%c\n",
            index, 
            NM(elf_prog_type_names, prog->type),
            prog->file_offset, prog->file_size,
            prog->virt_address, prog->phys_address, prog->memory_size,
            prog->align,
            prog->flags & PROG_FLAGS_READ ? 'R' : '.',
            prog->flags & PROG_FLAGS_WRITE ? 'W' : '.',
            prog->flags & PROG_FLAGS_EXECUTE ? 'X' : '.'
        );
    }
}

static void elf64_print_section_header(elf64_section_header *section, int index, char *names) {
    if (section == NULL) {
        printf("    S# Name                 Type        Offset      Size  Algn Flg\n");
        //          nn 12345678901234567890 12345678 123456789 123456789  1234 XXX
    } else {
        char *name = section->name == 0 ? "(none)" : (names == NULL ? "(unknown)" : names + section->name);
        printf("    %2d %-20s %-8s %9ld %9ld   %4ld %c%c%c\n",
            index, 
            name,
            NM(elf_section_type_names, section->type),
            section->file_offset,
            section->size,
            section->address_alignment,
            section->flags & SECTION_FLAGS_WRITE ? 'W' : '.',
            section->flags & SECTION_FLAGS_ALLOC ? 'A' : '.',
            section->flags & SECTION_FLAGS_EXECINSTR ? 'X' : '.'
        );
    }
}

static void elf64_print_symbol(elf64_sym *symbol, list *section_headers, buffer *section_names, buffer *symbol_names) {
    if (symbol == NULL) {
        printf("    Name                           Type     Bind     Section       Value      Size\n");
        //          123456789012345678901234567890 12345678 12345678 1234567890 00000000 123456789
    } else {
        char *sym_name = symbol->st_name == 0 ? "(none)" : (symbol_names == NULL ? "(unknown)" : symbol_names->buffer + symbol->st_name);
        char *sym_type = NM(elf_sym_type_names, ELF64_ST_TYPE(symbol->st_info));
        char *sym_bind = NM(elf_sym_bind_names, ELF64_ST_BIND(symbol->st_info));

        elf64_section_header *section = section_headers->v->get(section_headers, symbol->st_shndx);
        char *sect_name = section == NULL ? "" : (section_names == NULL ? "(unknown)" : section_names->buffer + section->name);

        printf("    %-30s %-8s %-8s %-10s %8lx %9ld\n",
            sym_name,
            sym_type, 
            sym_bind,
            sect_name,
            symbol->st_value, 
            symbol->st_size
        );
    }
}

static bool read_elf64_all_headers(FILE *f, elf64_header *file_header, list *program_headers, list *section_headers) {
    size_t bytes;
    elf64_offset offs;

    fseek(f, 0, SEEK_CUR);
    if ((bytes = fread(file_header, 1, sizeof(elf64_header), f)) < sizeof(elf64_header))
        return false;
    
    // sanity check before we go and load a bunch of garbage
    if (memcmp(file_header->identity, "\177ELF", 4) != 0)
        return false;
    
    if (file_header->prog_headers_offset > 0) {
        for (int i = 0; i < file_header->prog_headers_entries; i++) {
            long offs = file_header->prog_headers_offset + i  * file_header->prog_headers_entry_size;
            fseek(f, offs, SEEK_SET);
            elf64_prog_header *prog = malloc(sizeof(elf64_prog_header));
            if ((bytes = fread(prog, 1, sizeof(elf64_prog_header), f)) != sizeof(elf64_prog_header))
                return false;
            program_headers->v->add(program_headers, prog);
        }
    }

    if (file_header->section_headers_offset > 0) {
        for (int i = 0; i < file_header->section_headers_entries; i++) {
            long offs = file_header->section_headers_offset + i  * file_header->section_headers_entry_size;
            fseek(f, offs, SEEK_SET);
            elf64_section_header *sect = malloc(sizeof(elf64_section_header));
            if ((bytes = fread(sect, 1, sizeof(elf64_section_header), f)) != sizeof(elf64_section_header))
                return false;
            section_headers->v->add(section_headers, sect);
        }
    }

    return true;
}

static void elf64_print_headers(elf64_header *file_header, list *program_headers, list *section_headers, buffer *section_names_table) {
    int len;

    elf64_print_file_header(file_header);

    len = program_headers->v->length(program_headers);
    if (len > 0) {
        elf64_print_prog_header(NULL, 0);
        for (int i = 0; i < len; i++)
            elf64_print_prog_header(program_headers->v->get(program_headers, i), i);
    }

    len = section_headers->v->length(section_headers);
    if (len > 0) {
        elf64_print_section_header(NULL, 0, NULL);
        for (int i = 0; i < len; i++)
            elf64_print_section_header(section_headers->v->get(section_headers, i), i, section_names_table->buffer);
    }
}

static bool elf64_load_section(FILE *f, elf64_section_header *sect, buffer *buffer) {
    if (sect == NULL)
        return false;
    
    fseek(f, sect->file_offset, SEEK_SET);
    if (!buffer->from_file(buffer, f, sect->size))
        return false;
    
    return true;
}

static elf64_section_header *elf64_find_section_by_name(FILE *f, list *section_headers, buffer *names_table, char *name) {
    if (names_table == NULL)
        return NULL;
    
    for (int i = 0; i < section_headers->v->length(section_headers); i++) {
        elf64_section_header *sect = section_headers->v->get(section_headers, i);
        if (sect->name == 0)
            continue;
        
        char *sect_name = names_table->buffer + sect->name;
        if (strlen(sect_name) > 0 && strcmp(sect_name, name) == 0)
            return sect;
    }

    return NULL;
}

static bool read_elf64_file(FILE *f, elf_contents *contents) {
    size_t bytes;
    int i;
    elf64_offset offs;

    elf64_header *file_header = malloc(sizeof(elf64_header));
    list *section_headers = new_list();
    list *program_headers = new_list();

    // load all headers, to iterate as needed
    if (!read_elf64_all_headers(f, file_header, program_headers, section_headers)) {
        free(file_header);
        section_headers->v->free(section_headers, NULL);
        program_headers->v->free(program_headers, NULL);
        return false;
    }

    // load section names, in order to print sections
    buffer *section_names_table = new_buffer();
    if (file_header->section_headers_strings_entry > 0) {
        elf64_section_header *section_names = section_headers->v->get(section_headers, file_header->section_headers_strings_entry);
        elf64_load_section(f, section_names, section_names_table);
    }

    // print them all for debugging purposes
    elf64_print_headers(file_header, program_headers, section_headers, section_names_table);


    // debug symbol loading...
    elf64_section_header *strings_header = elf64_find_section_by_name(f, section_headers,section_names_table, ".strtab");
    elf64_section_header *symbols_header = elf64_find_section_by_name(f, section_headers,section_names_table, ".symtab");
    buffer *strings_table = new_buffer();
    buffer *symbols_table = new_buffer();
    elf64_load_section(f, strings_header, strings_table);
    elf64_load_section(f, symbols_header, symbols_table);

    if (symbols_table->length > 0) {
        int scount = symbols_table->length / sizeof(elf64_sym);
        printf("Symbols in file - total %d\n", scount);
        elf64_print_symbol(NULL, NULL, NULL, NULL);
        for (int i = 0; i < scount; i++) {
            elf64_sym *sym = (elf64_sym *)&symbols_table->buffer[i * sizeof(elf64_sym)];
            elf64_print_symbol(sym, section_headers, section_names_table, strings_table);
        }
    }
    /*
        Name                           Type     Bind     Section       Value      Size
        (none)                         NOTYPE   LOCAL                      0         0
        mcc.c                          FILE     LOCAL                      0         0
        (none)                         SECTION  LOCAL    .text             0         0
        run_unit_tests                 FUNC     LOCAL    .text             0        50  // <-- static func
        (none)                         SECTION  LOCAL    .rodata           0         0
        buffer_unit_tests              NOTYPE   GLOBAL                     0         0  // <-- extern
        string_unit_tests              NOTYPE   GLOBAL                     0         0
        list_unit_tests                NOTYPE   GLOBAL                     0         0
        unit_tests_outcome             NOTYPE   GLOBAL                     0         0
        load_source_code               FUNC     GLOBAL   .text            32       223  // <-- non static func
    */

    // debug relocations loading...

    // now selectively pick things and load into elf_contents...





    return true;
}

void read_elf_file(char *filename, elf_contents *contents) {
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
        read_elf32_file(f, contents);
    }
    if (buff[ELF_IDENTITY_CLASS] == ELF_CLASS_64) {
        read_elf64_file(f, contents);
    }
    printf("\n");
    fclose(f);
}

