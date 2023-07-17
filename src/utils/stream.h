#pragma once
#include "all.h"



typedef struct stream stream;

// can be memory based, file based, log, pipe, shared memory etc.
struct stream {
    bool (*reset)(stream *s);
    bool (*can_read)(stream *s);
    bool (*can_write)(stream *s);
    bool (*eof)(stream *s);

    // size is determining blob size
    bin *(*read_bin)(stream *s, mempool *mp, size_t bytes);
    bool (*write_bin)(stream *s, bin *buffer);

    // new line is implicit separator
    str *(*read_line)(stream *s, mempool *mp);
    bool (*write_line)(stream *s, str *line);

    void *private_data;
};

typedef enum stream_mode {
    SM_READ_ONLY,
    SM_WRITE_ONLY,
    SM_READ_WRITE,
    SM_APPEND_ONLY
} stream_mode;

stream *new_stream_file_based(mempool *mp, str *filename, stream_mode mode);
stream *new_stream_memory_based(mempool *mp, bin *buffer, stream_mode mode);
stream *new_stream_console_based(mempool *mp, stream_mode mode);

