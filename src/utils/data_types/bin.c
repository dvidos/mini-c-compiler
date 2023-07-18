#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "bin.h"
#include "str.h"


typedef struct bin {
    char *buffer;
    size_t capacity;
    size_t length;
    size_t position;
    mempool *mempool;
} bin;

bin *new_bin(mempool *mp) {
    bin *b = mpalloc(mp, bin);

    b->capacity = 16;
    b->buffer = mpallocn(mp, b->capacity, "binary buffer");
    b->length = 0;
    b->position = 0;
    b->mempool = mp;

    return b;
}

static void bin_ensure_capacity(bin *b, size_t capacity) {
    if (b->capacity >= capacity)
        return;
    
    while (b->capacity < capacity)
        b->capacity *= 2;
    
    char *old_buffer = b->buffer;
    b->buffer = mpallocn(b->mempool, b->capacity, "binary buffer");
    memcpy(b->buffer, old_buffer, b->length);
}

bin *new_bin_from_mem(mempool *mp, char *address, size_t size) {
    bin *b = mpalloc(mp, bin);
    memset(b, 0, sizeof(bin));

    b->capacity = size;
    b->buffer = mpallocn(mp, b->capacity, "binary buffer");
    memcpy(b->buffer, address, size);
    b->length = size;
    b->position = 0;
    b->mempool = mp;

    return b;
}

bin *new_bin_from_file(mempool *mp, str *filename) {
    FILE *f = fopen(str_charptr(filename), "rb");
    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    size_t file_length = ftell(f);
    fseek(f, 0, SEEK_SET);

    bin *b = mpalloc(mp, bin);
    memset(b, 0, sizeof(bin));

    b->capacity = file_length;
    b->buffer = mpallocn(mp, b->capacity, "binary buffer");

    fread(b->buffer, 1, file_length, f);
    fclose(f);

    b->length = file_length;
    b->position = 0;
    b->mempool = mp;

    return b;
}

bin *new_bin_from_stream(mempool *mp, FILE *stream, size_t offset, size_t length) {
    fseek(stream, offset, SEEK_SET);

    bin *b = new_bin(mp);
    bin_ensure_capacity(b, length);

    int bytes = fread(b->buffer, 1, length, stream);
    if (bytes != length)
        return NULL;

    b->length = length;
    return b;
}

bin *new_bin_from_zeros(mempool *mp, size_t size) {
    bin *b = mpalloc(mp, bin);
    memset(b, 0, sizeof(bin));

    b->capacity = size;
    b->buffer = mpallocn(mp, b->capacity, "binary buffer");
    memset(b->buffer, 0, size);
    b->length = size;
    b->position = 0;
    b->mempool = mp;

    return b;
}

size_t bin_len(bin *b) {
    return b->length;
}

void *bin_ptr_at(bin *b, size_t offset) {
    return offset < b->length ? b->buffer + offset : NULL;
}

void bin_clear(bin *b) {
    b->length = 0;
    b->position = 0;
}

int bin_cmp(bin *b1, bin *b2) {
    if (b1->length != b2->length)
        return b2->length - b1->length;
    
    return memcmp(b1->buffer, b2->buffer, b1->length);
}

void bin_cpy(bin *b, bin *source) {
    bin_clear(b);
    bin_cat(b, source);
}

void bin_cat(bin *b, bin *other) {
    bin_ensure_capacity(b, b->length + other->length);
    memcpy(b->buffer + b->length, other->buffer, other->length);
    b->length += other->length;
}

bin *bin_clone(bin *b, mempool *mp) {
    return new_bin_from_mem(mp, b->buffer, b->length);
}

void bin_pad(bin *b, char value, size_t target_len) {
    if (b->length >= target_len)
        return;
    
    bin_ensure_capacity(b, target_len);
    int gap = target_len - b->length;
    memset(b->buffer + b->length, value, gap);
    b->length += gap;
}

void bin_print_hex(bin *b, int indent, size_t offset, size_t length, FILE *f) {
    unsigned char *p = b->buffer + offset;
    char prep[64];

    if (length == -1)
        length = b->length;
    
    while (length > 0) {
        for (int i = 0; i < 16; i++) {
            if (i < length) {
                int c = (int)p[i];
                sprintf(&prep[i * 4], "%02x", c);
                prep[i*4  + 3] = (c >= ' ' && c <= '~') ? c : '.';
            } else {
                strcpy(&prep[i * 4], "  ");
                prep[i*4  + 3] = ' ';
            }
        }

        printf("%*s%08lx   %2s %2s %2s %2s %2s %2s %2s %2s  %2s %2s %2s %2s %2s %2s %2s %2s   %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
            indent, "", offset,
            prep +  0, prep +  4, prep +  8, prep + 12, 
            prep + 16, prep + 20, prep + 24, prep + 28, 
            prep + 32, prep + 36, prep + 40, prep + 44, 
            prep + 48, prep + 52, prep + 56, prep + 60, 
            prep[ 3], prep[ 7], prep[11], prep[15], 
            prep[19], prep[23], prep[27], prep[31], 
            prep[35], prep[39], prep[43], prep[47], 
            prep[51], prep[55], prep[59], prep[63]
        );

        p += 16;
        offset += 16;
        length -= (length >= 16) ? 16 : length; // length cannot go negative
    }

}

// emulate a file a bit? i think it's useful to maintain internal pointer
void bin_seek(bin *b, size_t offset) {
    if (offset > b->length)
        offset = b->length;
    b->position = offset;
}

size_t  bin_tell(bin *b) {
    return b->position;
}

// all "read" funcs work at current offset, they advance offset
u8 bin_read_byte(bin *b) {
    if (b->position >= b->length)
        return 0;
    
    u8 value = *(u8 *)(b->buffer + b->position);
    b->position += 1;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u16  bin_read_word(bin *b) {
    if (b->position >= b->length)
        return 0;
    
    u16 value = *(u16 *)(b->buffer + b->position);
    b->position += 2;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u32  bin_read_dword(bin *b) {
    if (b->position >= b->length)
        return 0;
    
    u32 value = *(u32 *)(b->buffer + b->position);
    b->position += 4;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u64  bin_read_qword(bin *b) {
    if (b->position >= b->length)
        return 0;
    
    u64 value = *(u64 *)(b->buffer + b->position);
    b->position += 8;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

void bin_read_mem(bin *b, void *ptr, size_t length) {
    if (b->position >= b->length)
        return;
    
    memcpy(ptr, b->buffer + b->position, length);
    b->position += length;
    if (b->position > b->length)
        b->position = b->length;
}

// all "write" funcs work at current offset, they advance offset
void bin_write_byte(bin *b, u8 value) {
    bin_ensure_capacity(b, b->position + 1);
    b->buffer[b->position] = value;
    b->position += 1;
    if (b->position > b->length)
        b->length = b->position;
}

void bin_write_word(bin *b, u16 value) {
    bin_ensure_capacity(b, b->position + sizeof(u16));
    *(u16 *)(b->buffer + b->position) = value;
    b->position += sizeof(u16);
    if (b->position > b->length)
        b->length = b->position;
}

void bin_write_dword(bin *b, u32 value) {
    bin_ensure_capacity(b, b->position + sizeof(u32));
    *(u32 *)(b->buffer + b->position) = value;
    b->position += sizeof(u32);
    if (b->position > b->length)
        b->length = b->position;
}

void bin_write_qword(bin *b, u64 value) {
    bin_ensure_capacity(b, b->position + sizeof(u64));
    *(u64 *)(b->buffer + b->position) = value;
    b->position += sizeof(u64);
    if (b->position > b->length)
        b->length = b->position;
}

void bin_write_mem(bin *b, const void *ptr, size_t length) {
    bin_ensure_capacity(b, b->position + length);
    memcpy(b->buffer + b->position, ptr, length);
    b->position += length;
    if (b->position > b->length)
        b->length = b->position;
}

void bin_write_zeros(bin *b, size_t length) {
    bin_ensure_capacity(b, b->position + length);
    memset(b->buffer + b->position, 0, length);
    b->position += length;
    if (b->position > b->length)
        b->length = b->position;
}

// these implicitely append data at the end of the buffer
void bin_add_byte(bin *b, u8 value) {
    b->position = b->length;
    bin_write_byte(b, value);
}

void bin_add_word(bin *b, u16 value)  {
    b->position = b->length;
    bin_write_word(b, value);
}

void bin_add_dword(bin *b, u32 value) {
    b->position = b->length;
    bin_write_dword(b, value);
}

void bin_add_qword(bin *b, u64 value) {
    b->position = b->length;
    bin_write_qword(b, value);
}

void bin_add_mem(bin *b, const void *ptr, size_t length) {
    b->position = b->length;
    bin_write_mem(b, ptr, length);
}

void bin_add_str(bin *b, str *str) {
    b->position = b->length;
    bin_write_mem(b, str_charptr(str), str_len(str) + 1);
}

void bin_add_zeros(bin *b, size_t length) {
    b->position = b->length;
    bin_write_zeros(b, length);
}

int  bin_index_of(bin *b, const void *ptr, size_t size) {
    if (size > b->length)
        return -1;
    
    // naive approach for now
    for (int i = 0; i <= b->length - size; i++) {
        if (memcmp(b->buffer + i, ptr, size) == 0)
            return i;
    }

    return -1;
}

bin *bin_slice(bin *b, size_t offset, size_t size, mempool *mp) {
    return new_bin_from_mem(mp, b->buffer + offset, size);
}

str *bin_str(bin *b, size_t offset, mempool *mp) {
    // hoping this ends in a zero
    return new_str(mp, b->buffer + offset);
}

bool bin_save_to_file(bin *b, str *filename) {
    FILE *f = fopen(str_charptr(filename), "wb");
    if (f == NULL)
        return false;

    fwrite(b->buffer, 1, b->length, f);
    fclose(f);
    return true;
}

str *bin_to_readable_bytes_str(bin *b, mempool *mp) {
    str *s = new_str(mp, NULL);
    for (int i = 0; i < b->length; i++) {
        if (i > 0) str_cats(s, ", ");
        str_catf(s, "0x%02x", (unsigned char)b->buffer[i]);
    }
    return s;
}


#ifdef INCLUDE_UNIT_TESTS
void bin_unit_tests() {
    mempool *mp = new_mempool();
    bin *b;
    bin *b1;
    str *s;

    char *bytes32 = mpallocn(mp, 32, "bytes32");
    for (int i = 0; i < 32; i++)
        bytes32[i] = (char)i;

    char *mem = mpallocn(mp, 16, "mem");

    b = new_bin(mp);
    assert(b != NULL);
    assert(bin_len(b) == 0);

    b = new_bin_from_zeros(mp, 6);
    assert(b != NULL);
    assert(bin_len(b) == 6);
    assert(memcmp(b->buffer, "\0\0\0\0\0\0", 6) == 0);
    assert(bin_ptr_at(b, 0) == b->buffer);
    assert(bin_ptr_at(b, 3) == b->buffer + 3);
    assert(bin_ptr_at(b, 9999) == NULL);

    b = new_bin_from_mem(mp, "\x12\x34\x56\x78", 4);
    assert(b != NULL);
    assert(bin_len(b) == 4);
    assert(memcmp(b->buffer, "\x12\x34\x56\x78", 4) == 0);
    bin_clear(b);
    assert(bin_len(b) == 0);

    bin_clear(b);
    bin_add_zeros(b, 16);
    assert(bin_len(b) == 16);
    assert(memcmp(b->buffer, "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_add_byte(b, 0x12);
    assert(bin_len(b) == 1);
    assert(memcmp(b->buffer, "\x12", 1) == 0);

    bin_clear(b);
    bin_add_word(b, 0x1234);
    assert(bin_len(b) == 2);
    assert(memcmp(b->buffer, "\x34\x12", 2) == 0);

    bin_clear(b);
    bin_add_dword(b, 0x12345678);
    assert(bin_len(b) == 4);
    assert(memcmp(b->buffer, "\x78\x56\x34\x12", 4) == 0);

    bin_clear(b);
    bin_add_qword(b, 0x123456789ABCDEF0);
    assert(bin_len(b) == 8);
    assert(memcmp(b->buffer, "\xF0\xDE\xBC\x9A\x78\x56\x34\x12", 8) == 0);

    bin_clear(b);
    bin_add_mem(b, "\x01\x02", 2);
    assert(bin_len(b) == 2);
    assert(memcmp(b->buffer, "\x01\x02", 2) == 0);

    bin_clear(b);
    bin_add_str(b, new_str(mp, "name"));
    assert(bin_len(b) == 5);
    assert(memcmp(b->buffer, "name\0", 5) == 0);

    b = new_bin_from_mem(mp, "\x12\x34", 2);
    b1 = new_bin_from_mem(mp, "\x12\x34", 2);
    assert(b != b1);
    assert(memcmp(b->buffer, b1->buffer, 2) == 0);
    assert(bin_cmp(b, b1) == 0);

    bin_cat(b, b1);
    assert(bin_len(b) == 4);
    assert(memcmp(b->buffer, "\x12\x34\x12\x34", 4) == 0);

    b1 = bin_clone(b, mp);
    assert(b != b1);
    assert(bin_cmp(b, b1) == 0);

    b = new_bin_from_mem(mp, "\x12\x34", 2);
    assert(bin_len(b) == 2);
    bin_pad(b, 0x55, 4);
    assert(bin_len(b) == 4);
    assert(memcmp(b->buffer, "\x12\x34\x55\x55", 4) == 0);

    // index_of
    b = new_bin_from_mem(mp, bytes32, 32);
    assert(bin_index_of(b, "\x06\x07\x08", 3) == 6);
    assert(bin_index_of(b, "\x06\x06\x06", 3) == -1);
    assert(bin_index_of(b, "", 0) == 0);

    // get_slice
    b = new_bin_from_mem(mp, bytes32, 32);
    b1 = bin_slice(b, 4, 5, mp);
    assert(b1 != NULL);
    assert(bin_len(b1) == 5);
    assert(memcmp(b1->buffer, "\x04\x05\x06\x07\x08", 5) == 0);

    // get_str
    b = new_bin_from_mem(mp, "\0abc\0def\0ghi\0", 13);
    s = bin_str(b, 5, mp);
    assert(s != NULL);
    assert(str_len(s) == 3);
    assert(str_cmps(s, "def") == 0);

    // check read/write
    b = new_bin_from_mem(mp, bytes32, 32);
    assert(bin_tell(b) == 0);

    bin_seek(b, 10);
    assert(bin_tell(b) == 10);
    assert(bin_read_byte(b) == 0x0A);
    assert(bin_tell(b) == 11);
    
    bin_seek(b, 16);
    assert(bin_tell(b) == 16);
    assert(bin_read_word(b) == 0x1110);
    assert(bin_tell(b) == 18);
    
    bin_seek(b, 8);
    assert(bin_tell(b) == 8);
    assert(bin_read_dword(b) == 0x0b0a0908);
    assert(bin_tell(b) == 12);
    
    bin_seek(b, 4);
    assert(bin_tell(b) == 4);
    assert(bin_read_qword(b) == 0x0b0a090807060504);
    assert(bin_tell(b) == 12);
    
    bin_seek(b, 4);
    assert(bin_tell(b) == 4);
    bin_read_mem(b, mem, 8);
    assert(memcmp(mem, "\x04\x05\x06\x07\x08\x09\x0a\x0b", 8) == 0);
    assert(bin_tell(b) == 12);

    bin_clear(b);
    bin_add_zeros(b, 16);
    bin_seek(b, 4);
    bin_write_byte(b, 0x12);
    assert(bin_tell(b) == 5);
    assert(memcmp(b->buffer, "\0\0\0\0\x12\0\0\0\0\0\0\0\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_add_zeros(b, 16);
    bin_seek(b, 4);
    bin_write_word(b, 0x1234);
    assert(bin_tell(b) == 6);
    assert(memcmp(b->buffer, "\0\0\0\0\x34\x12\0\0\0\0\0\0\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_add_zeros(b, 16);
    bin_seek(b, 4);
    bin_write_dword(b, 0x12345678);
    assert(bin_tell(b) == 8);
    assert(memcmp(b->buffer, "\0\0\0\0\x78\x56\x34\x12\0\0\0\0\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_add_zeros(b, 16);
    bin_seek(b, 4);
    bin_write_qword(b, 0x0123456789abcdef);
    assert(bin_tell(b) == 12);
    assert(memcmp(b->buffer, "\0\0\0\0\xef\xcd\xab\x89\x67\x45\x23\x01\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_add_zeros(b, 16);
    bin_seek(b, 4);
    bin_write_mem(b, bytes32, 5);
    assert(bin_tell(b) == 9);
    assert(memcmp(b->buffer, "\0\0\0\0\x00\x01\x02\x03\x04\0\0\0\0\0\0\0", 16) == 0);

    bin_clear(b);
    bin_pad(b, 0xff, 16);
    bin_seek(b, 4);
    bin_write_zeros(b, 5);
    assert(bin_tell(b) == 9);
    assert(memcmp(b->buffer, "\xff\xff\xff\xff\0\0\0\0\0\xff\xff\xff\xff\xff\xff\xff", 16) == 0);

    // save, load
    char *fname = mpallocn(mp, 50, "fname");
    strcpy(fname, "/tmp/temp_XXXXXX");
    mkstemp(fname);
    b = new_bin_from_mem(mp, bytes32, 32);
    bool saved = bin_save_to_file(b, new_str(mp, fname));
    assert(saved);
    // one hour later...
    bin_clear(b);
    b = new_bin_from_file(mp, new_str(mp, fname));
    assert(b != NULL);
    assert(bin_len(b) == 32);
    assert(memcmp(b->buffer, bytes32, 32) == 0);
    unlink(fname);
}

#endif
