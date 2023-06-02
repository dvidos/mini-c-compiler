#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "mempool.h"
#include "data_structs.h"
#include "unit_tests.h"

// ----------------------------------------

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t   s8;
typedef int16_t  s16;
typedef int32_t  s32;
typedef int64_t  s64;

typedef struct str str;
typedef struct llist llist;
typedef struct iterator iterator;


// --------------------------------------------

typedef struct pair {
    str *key;
    void *value;
} pair;

pair *new_pair(mempool *mp, str *key, void *value);

// ---------------------------------------------

str *new_str(mempool *mp, const char *str);
str *new_strf(mempool *mp, const char *fmt, ...);
str *new_str_random(mempool *mp, int min_len, int max_len);
int  str_len(str *s);
bool str_is_empty(str *s);
void str_clear(str *s);
bool str_starts_with(str *s, str *fragment);
bool str_ends_with(str *s, str *fragment);
bool str_contains(str *s, str *fragment);
bool str_contains_char(str *s, char c);
str *str_left(str *s, int len);
str *str_right(str *s, int len);
str *str_substr(str *s, int start, int len);
str *str_toupper(str *s);
str *str_tolower(str *s);
char str_char_at(str *s, int index);
int  str_index_of(str *s, str *needle, int start);
int  str_index_of_any(str *s, str *characters, int start);
int  str_last_index_of(str *s, str *needle);
int  str_char_pos(str *s, char c, int start);
int  str_last_char_pos(str *s, char c);
str *str_replace(str *s, str *needle, str *replacement);
str *str_replace_chars(str *s, str *characters, str *replacement);
str *str_trim(str *s, str *characters);
str *str_padr(str *s, int len, char c);
str *str_padl(str *s, int len, char c);
void str_cat(str *s1, str *s2);
void str_cats(str *s1, char *s2);
void str_catc(str *s, char c);
int  str_cmp(str *s1, str *s2);
int  str_cmps(str *s1, char *s2);
bool str_equals(str *s1, str *s2);
unsigned int str_hash(str *s);
llist *str_split(str *s, str *delimiters, bool include_empty_tokens, int max_items, mempool *mp);
str *str_join(llist *strings, str *delimiter, mempool *mp);
str *str_clone(str *s);
bool str_save_file(str *s, str *filename);
str *str_load_file(str *filename, mempool *mp);
const char *str_charptr(str *s);

// -------------------------------------------

typedef struct bin bin;

bin *new_bin(mempool *mp);
bin *new_bin_from_mem(mempool *mp, char *address, size_t size);
bin *new_bin_from_file(mempool *mp, str *filename);
bin *new_bin_from_zeros(mempool *mp, size_t size);

size_t bin_len(bin *b);
void  *bin_ptr_at(bin *b, size_t offset);
void   bin_clear(bin *b);
int    bin_cmp(bin *b1, bin *b2);
void   bin_cat(bin *b, bin *other);
bin   *bin_clone(bin *b, mempool *mp);
void   bin_pad(bin *b, char value, size_t target_len);
void   bin_print_hex(bin *b, FILE *f);
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

// -------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests();
#endif

