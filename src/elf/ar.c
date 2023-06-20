#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"
#include "ar.h"

#define AR_HEADER_BYTES   8

archive *ar_open(mempool *mp, str *filename) {
    archive *a = mempool_alloc(mp, sizeof(archive), "archive");
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
    if (memcmp(file_header, "!<arch>\x0A", AR_HEADER_BYTES) != 0)
        return NULL;

    return a;
}

static bool ar_load_entry_header(archive *a, char *filename_buffer16, size_t *offset_ptr, long *size_ptr) {
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

static bin *ar_load_short_named_entry(archive *a, char *name) {
    // assuming it's in the first three entries
    char filename[16+1];
    long offset, size;
    fseek(a->handle, AR_HEADER_BYTES, SEEK_SET);
    while (true) {
        if (!ar_load_entry_header(a, filename, &offset, &size))
            break;
        if (strcmp(filename, name) == 0)
            return new_bin_from_stream(a->mempool, a->handle, offset, size);
        fseek(a->handle, size + (size & 1), SEEK_CUR);
    }
    return NULL; // not found
}

static str *get_long_file_name(archive *a, bin *long_names_table, long offset) {
    char *start = bin_ptr_at(long_names_table, offset);
    char *end = strchr(start, 0x0A);
    return new_str_from_mem(a->mempool, start, end - start);
}

llist *ar_get_entries(archive *a) {
    char shortname[16+1];
    long offset;
    long size;
    str *filename;
    str *slash;
    str *empty;

    slash = new_str(a->mempool, "/");
    empty = new_str(a->mempool, "");
    bin *long_names_table = ar_load_short_named_entry(a, "//");
    llist *l = new_llist(a->mempool);

    int i = 0; 
    fseek(a->handle, AR_HEADER_BYTES, SEEK_SET);
    while (ar_load_entry_header(a, shortname, &offset, &size)) {
        if (strcmp(shortname, "/") == 0 || strcmp(shortname, "//") == 0) {
            fseek(a->handle, size + (size & 1), SEEK_CUR);
            continue;
        }
        
        // long filenames are in the form of "/nnn"
        // this is the offset of the long name in the table.
        // short filenames end in a slash e.g. "lc-paper.o/"
        if (shortname[0] == '/' && (shortname[1] >= '0' && shortname[1] <= '9'))
            filename = get_long_file_name(a, long_names_table, atol(shortname + 1));
        else
            filename = new_str(a->mempool, shortname);
        filename = str_replace(filename, slash, empty);

        // printf(" -- short name '%s',%*s long name '%s'\n", shortname, (int)(16 - strlen(shortname)), "", str_charptr(filename));
        archive_entry *e = mempool_alloc(a->mempool, sizeof(archive_entry), "archive_entry");
        e->filename = filename;
        e->offset = offset;
        e->size = size;
        llist_add(l, e);

        fseek(a->handle, size + (size & 1), SEEK_CUR);
    }

    return l;
}

llist *ar_get_symbols(archive *a) {
    unsigned char buff[4];

    bin *symbols_table = ar_load_short_named_entry(a, "/");
    if (symbols_table == NULL)
        return NULL;
    
    llist *symbols_list = new_llist(a->mempool);

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
    // so they are something like an offset to the archive.
    // we are supposed to parse the names in parallel, without a name being pointed by somewhere.
    for (int i = 0; i < symbols_count; i++) {
        bin_seek(symbols_table, 4 + i * 4);
        bin_read_mem(symbols_table, buff, 4); 

        archive_symbol *s = mempool_alloc(a->mempool, sizeof(archive_symbol), "archive_symbol");
        s->name = bin_str(symbols_table, name_offset, a->mempool);
        s->entry_header_offset = (buff[0] << 24) + (buff[1] << 16) + (buff[2] << 8) + buff[3];

        llist_add(symbols_list, s);
        name_offset += str_len(s->name) + 1;
    }

    return symbols_list;
}

bin *ar_read_file(archive *a, archive_entry *e) {
    return new_bin_from_stream(a->mempool, a->handle, e->offset, e->size);
}

void ar_print_entries(llist *entries, int max_entries, FILE *stream) {
    mempool *mp = new_mempool();

    iterator *it = llist_create_iterator(entries, mp);
    fprintf(stream, "   Idx      Offset        Size  File name\n");
    //              "  1234  1234567890  1234567890  123..."

    int idx = 0;
    for_iterator(archive_entry, e, it) {
        fprintf(stream, "  %4d %10ld  %10ld  %s\n", idx++, e->offset, e->size, str_charptr(e->filename));
        if (max_entries > -1 && idx > max_entries)
            break;
    }

    mempool_release(mp);
}

void ar_close(archive *a) {
    fclose(a->handle);
    a->handle = NULL;
}





