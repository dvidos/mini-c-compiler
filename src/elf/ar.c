#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/all.h"
#include "ar.h"

#define AR_HEADER_BYTES                  8
#define AR_HEADER_VALUE                  "!<arch>\x0A"
#define SYMBOLS_TABLE_ENTRYNAME          "/"
#define LONG_FILENAMES_TABLE_ENTRYNAME   "//"


archive *ar_open(mempool *mp, str *filename) {
    archive *a = mpalloc(mp, archive);
    a->mempool = mp;
    a->filename = new_str(a->mempool, str_charptr(filename));

    a->handle = fopen(str_charptr(filename), "r");
    if (a->handle == NULL)
        return NULL;

    // validate header
    char file_header[AR_HEADER_BYTES];
    size_t read = fread(file_header, 1, sizeof(file_header), a->handle);
    if (read != sizeof(file_header))
        return NULL;
    if (memcmp(file_header, AR_HEADER_VALUE, AR_HEADER_BYTES) != 0)
        return NULL;

    return a;
}

static bool load_entry_header_bytes(archive *a, char *filename_buffer16, size_t *offset_ptr, long *size_ptr) {
    char entry_header[60];
    char size_buffer[10+1];
    size_t offset;

    if (feof(a->handle))
        return false;
    
    int read = fread(entry_header, 1, sizeof(entry_header), a->handle);
    if (read != sizeof(entry_header))
        return false;

    *offset_ptr = ftell(a->handle);

    // size   name
    //   16   file name "/" is end indicator, padded with spaces
    //   12   modification 12 bytes
    //    6   owner id
    //    6   group id
    //    8   file mode
    //   10   file size
    //    2   terminating chars (0x60 0x0A)

    memcpy(filename_buffer16, entry_header, 16);
    filename_buffer16[16] = 0;

    for (int i = 15; i >= 0; i--)
        if (filename_buffer16[i] == ' ')
            filename_buffer16[i] = 0;
    
    memcpy(size_buffer, entry_header +16+12+6+6+8, 10);
    size_buffer[10] = 0;
    *size_ptr = atol(size_buffer);

    return true;
}

static bin *load_short_named_entry_contents(archive *a, char *name) {
    char filename[16+1];
    long offset, size;

    // go over the file and look for the specific entry name
    fseek(a->handle, AR_HEADER_BYTES, SEEK_SET);
    while (true) {
        if (!load_entry_header_bytes(a, filename, &offset, &size))
            break;
        if (strcmp(filename, name) == 0)
            return new_bin_from_stream(a->mempool, a->handle, offset, size);
        fseek(a->handle, size + (size & 1), SEEK_CUR);
    }
    return NULL; // not found
}

static str *resolve_entry_filename(char *shortname, bin *long_names_table, mempool *mp) {
    str *filename;

    // these two are special names, they contain tables and long names
    if (strcmp(shortname, "/") == 0 || strcmp(shortname, "//") == 0)
        return new_str(mp, shortname);

    // long filenames are in the form of "/nnn"
    // this is the offset of the long name in the table.
    // short filenames end in a slash e.g. "lc-paper.o/"
    if (long_names_table != NULL 
        && shortname[0] == '/' 
        && (shortname[1] >= '0' && shortname[1] <= '9'))
    {
        char *start = bin_ptr_at(long_names_table, atol(shortname + 1));
        char *end = strchr(start, 0x0A);
        filename = new_str_from_mem(mp, start, end - start);
    } else {
        filename = new_str(mp, shortname);
    }

    if (str_char_at(filename, str_len(filename) - 1) == '/')
        filename = str_substr(filename, 0, -1);

    // printf(" -- short name '%s',%*s long name '%s'\n", shortname, (int)(16 - strlen(shortname)), "", str_charptr(filename));
    return filename;
}

static archive_entry *get_entry_header_at(archive *a, size_t header_offset, bin *long_names_table, mempool *mp) {

    char shortname[16+1];
    long offset;
    long size;
    fseek(a->handle, header_offset, SEEK_SET);
    if (!load_entry_header_bytes(a, shortname, &offset, &size))
        return NULL;
    
    archive_entry *e = mpalloc(mp, archive_entry);
    e->filename = resolve_entry_filename(shortname, long_names_table, mp);
    e->offset = offset;
    e->size = size;

    return e;
}

list *ar_get_entries(archive *a, mempool *mp) {
    archive_entry *entry;

    bin *long_names_table = load_short_named_entry_contents(a, LONG_FILENAMES_TABLE_ENTRYNAME);
    list *entries = new_list(mp);

    fseek(a->handle, AR_HEADER_BYTES, SEEK_SET);
    while ((entry = get_entry_header_at(a, ftell(a->handle), long_names_table, mp)) != NULL) {
        if (str_cmps(entry->filename, SYMBOLS_TABLE_ENTRYNAME) == 0 || 
            str_cmps(entry->filename, LONG_FILENAMES_TABLE_ENTRYNAME) == 0)
        {
            fseek(a->handle, entry->size + (entry->size & 1), SEEK_CUR);
            continue;
        }

        list_add(entries, entry);
        fseek(a->handle, entry->size + (entry->size & 1), SEEK_CUR);
    }

    return entries;
}

list *ar_get_symbols(archive *a, mempool *mp) {
    unsigned char buff[4];
    unsigned long offset;
    unsigned long last_offset = -1;
    archive_entry *last_entry = NULL;

    bin *symbols_table = load_short_named_entry_contents(a, SYMBOLS_TABLE_ENTRYNAME);
    if (symbols_table == NULL)
        return NULL; // there is no symbols table
    
    bin *long_names_table = load_short_named_entry_contents(a, LONG_FILENAMES_TABLE_ENTRYNAME);
    list *symbols_list = new_list(mp);

    // in the "/" entries,
    // 4 bytes big endian the number of symbols (00 00 11 d5 = 4565 symbols)
    // then, as many sets of 4-bytes follow (4565 * 4 = 18260)
    // but we also have the 8 bytes of "<arch>" and the 60 bytes of the "/" file entry, and 4 bytes the number of symbols
    // so we go to 18332. there, a null-terminated string table starts

    // find number of symbols: four bytes big endian
    bin_seek(symbols_table, 0);
    bin_read_mem(symbols_table, buff, 4); 
    size_t symbols_count = (buff[0] << 24) + (buff[1] << 16) + (buff[2] << 8) + buff[3];
    size_t name_offset = 4 + symbols_count * 4;
    
    // the offsets are the same for all the symbols in the same module,
    // they are the offset to the entry header (e.g. the first would be at byte 8)
    // we are supposed to parse the names in parallel, without a name being pointed by somewhere.
    for (int i = 0; i < symbols_count; i++) {
        bin_seek(symbols_table, 4 + i * 4);
        bin_read_mem(symbols_table, buff, 4); 

        offset = (buff[0] << 24) + (buff[1] << 16) + (buff[2] << 8) + buff[3];
        if (offset != last_offset || last_entry == NULL) {
            last_entry = get_entry_header_at(a, offset, long_names_table, mp);
        }
        
        archive_symbol *s = mpalloc(mp, archive_symbol);
        s->name = bin_str(symbols_table, name_offset, a->mempool);
        s->entry = last_entry;

        list_add(symbols_list, s);
        last_offset = offset;
        name_offset += str_len(s->name) + 1;
    }

    return symbols_list;
}

bin *ar_load_file_contents(archive *a, archive_entry *e) {
    return new_bin_from_stream(a->mempool, a->handle, e->offset, e->size);
}

void ar_print_entries(list *entries, int max_entries, FILE *stream) {
    mempool *mp = new_mempool();

    iterator *it = list_create_iterator(entries, mp);
    fprintf(stream, "   Idx      Offset        Size  File name\n");
    //              "  1234  1234567890  1234567890  123..."

    int idx = 0;
    for_iterator(archive_entry, e, it) {
        fprintf(stream, "  %4d  %10ld  %10ld  %s\n", idx++, e->offset, e->size, str_charptr(e->filename));
        if (max_entries > -1 && idx > max_entries)
            break;
    }

    mempool_release(mp);
}

void ar_print_symbols(list *symbols, int max_symbols, FILE *stream) {
    mempool *mp = new_mempool();

    iterator *it = list_create_iterator(symbols, mp);
    fprintf(stream, "  Symbol name                    File                     Offset       Size\n");
    //              "  123456789012345678901234567890 12345678901234567890 1234567890 1234567890"

    int idx = 0;
    for_iterator(archive_symbol, s, it) {
        fprintf(stream, "  %-30s %-20s %10ld %10ld\n", 
            str_charptr(s->name), str_charptr(s->entry->filename), s->entry->offset, s->entry->size);
        if (max_symbols > -1 && ++idx > max_symbols)
            break;
    }

    mempool_release(mp);
}

void ar_close(archive *a) {
    fclose(a->handle);
    a->handle = NULL;
}





