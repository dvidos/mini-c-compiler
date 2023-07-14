#pragma once
#include <stdio.h>
#include "../utils/all.h"


// now, we want to open and use ar files.

typedef struct archive {
    str *filename;
    FILE *handle;
    size_t position;
    mempool *mempool;
} archive;

typedef struct archive_entry {
    str *filename;
    size_t offset;
    size_t size;
} archive_entry;

typedef struct archive_symbol {
    str *name;
    archive_entry *entry;
} archive_symbol;

archive *ar_open(mempool *mp, str *filename);
list *ar_get_entries(archive *a, mempool *mp); // list items are of type "archive_entry"
list *ar_get_symbols(archive *a, mempool *mp); // list items are of type "archive_symbol"
bin *ar_load_file_contents(archive *a, archive_entry *e);
void ar_print_entries(list *entries, int max_entries, FILE *stream);
void ar_print_symbols(list *symbols, int max_symbols, FILE *stream);
void ar_close(archive *a);





