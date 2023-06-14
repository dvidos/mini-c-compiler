#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "../utils/data_types.h"
#include "../utils/data_structs.h"
#include "ar.h"


archive *ar_open(mempool *mp, str *filename) {
    archive *a = mempool_alloc(mp, sizeof(archive), "archive");
    a->mempool = mp;
    a->filename = new_str(a->mempool, str_charptr(filename));

    a->handle = fopen(str_charptr(filename), "r");
    if (a->handle == NULL)
        return NULL;

    // validate header
    char file_header[8];
    size_t read = fread(file_header, 1, sizeof(file_header), a->handle);
    if (read != sizeof(file_header))
        return NULL;
    if (memcmp(file_header, "!<arch>\x0A", 8) != 0)
        return NULL;

    return a;
}

llist *ar_get_entries(archive *a) {
    char entry_header[60];
    char file_size_decimal[10+1];
    mempool *mp = new_mempool();

    str *trim_chars = new_str(mp, " /");
    str *single_slash_and_space = new_str(mp, "/ ");
    str *double_slash_and_space = new_str(mp, "// ");

    llist *l = new_llist(a->mempool);
    size_t pos = 8;
    while (true) {
        fseek(a->handle, pos, SEEK_SET);
        if (feof(a->handle))
            break;

        int read = fread(entry_header, 1, sizeof(entry_header), a->handle);
        if (read != sizeof(entry_header))
            break;
        pos += sizeof(entry_header);
        
        // size   name
        //   16   file name "/" is end indicator, padded with spaces
        //   12   modification 12 bytes
        //    6   owner id
        //    6   group id
        //    8   file mode
        //   10   file size
        //    2   terminating chars (0x60 0x0A)

        archive_entry *e = mempool_alloc(a->mempool, sizeof(archive_entry), "archive_entry");
        e->filename = new_str_from_mem(a->mempool, entry_header, 16);
        if (str_starts_with(e->filename, single_slash_and_space))
            e->filename = new_str(a->mempool, "/");
        else if (str_starts_with(e->filename, double_slash_and_space))
            e->filename = new_str(a->mempool, "//");
        else
            e->filename = str_trim(e->filename, trim_chars);

        memcpy(file_size_decimal, entry_header +16+12+6+6+8, 10);
        file_size_decimal[10] = 0;
        e->offset = pos;
        e->size = atol(file_size_decimal);

        llist_add(l, e);
        pos += e->size + ((e->size & 1) == 1 ? 1 : 0);  // contents are padded to two bytes boundaries
    }

    return l;
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





