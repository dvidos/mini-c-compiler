#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"


typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef struct str str;
typedef struct list list;
typedef struct iterator iterator;

typedef struct bin bin;

bin *new_bin(mempool *mp);
bin *new_bin_from_mem(mempool *mp, char *address, size_t size);
bin *new_bin_from_file(mempool *mp, str *filename);
bin *new_bin_from_stream(mempool *mp, FILE *stream, size_t offset, size_t length);
bin *new_bin_from_zeros(mempool *mp, size_t size);

size_t bin_len(bin *b);
void  *bin_ptr_at(bin *b, size_t offset);
void   bin_clear(bin *b);
int    bin_cmp(bin *b1, bin *b2);
void   bin_cpy(bin *b, bin *source);
void   bin_cat(bin *b, bin *other);
bin   *bin_clone(bin *b, mempool *mp);
void   bin_pad(bin *b, char value, size_t target_len);
void   bin_print_hex(bin *b, int indent, size_t offset, size_t length, FILE *f);
// these manipulate the read/write pointer, like a file
void   bin_seek(bin *b, size_t offset); // emulate 
size_t bin_tell(bin *b);
// all "read" funcs work at current offset, they advance offset
u8     bin_read_byte(bin *b);
u16    bin_read_word(bin *b);
u32    bin_read_dword(bin *b);
u64    bin_read_qword(bin *b);
void   bin_read_mem(bin *b, void *ptr, size_t length);
// all "write" funcs work at current offset, they advance offset
void   bin_write_byte(bin *b, u8 value);
void   bin_write_word(bin *b, u16 value);
void   bin_write_dword(bin *b, u32 value);
void   bin_write_qword(bin *b, u64 value);
void   bin_write_mem(bin *b, const void *ptr, size_t length);
void   bin_write_zeros(bin *b, size_t length);
// these implicitely append data at the end of the buffer
void   bin_add_byte(bin *b, u8 value);
void   bin_add_word(bin *b, u16 value);
void   bin_add_dword(bin *b, u32 value);
void   bin_add_qword(bin *b, u64 value);
void   bin_add_mem(bin *b, const void *ptr, size_t length);
void   bin_add_str(bin *b, str *str);
void   bin_add_zeros(bin *b, size_t length);

int  bin_index_of(bin *b, const void *ptr, size_t size);
bin *bin_slice(bin *b, size_t offset, size_t size, mempool *mp);
str *bin_str(bin *b, size_t offset, mempool *mp);
bool bin_save_to_file(bin *b, str *filename);
str *bin_to_readable_bytes_str(bin *b, mempool *mp);


#ifdef INCLUDE_UNIT_TESTS
void bin_unit_tests();
#endif

