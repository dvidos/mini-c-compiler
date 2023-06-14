#pragma once
#include "../utils/data_types.h"


// now, we want to open and use ar files.

typedef struct archive {
    FILE *file;
    size_t position;
    mempool *mempool;
} archive;

typedef struct archive_entry {
    str *filename;
    size_t offset;
    size_t size;
} archive_entry;


archive *ar_open(mempool *mp, str *filename);
llist *ar_get_entries(archive *a); // list items are of type "archive_entry"
bin *ar_read_file(archive *a, archive_entry *e);
void ar_close(archive *a);





