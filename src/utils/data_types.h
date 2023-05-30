#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "mempool.h"
#include "data_structs.h"
#include "../unit_tests.h"

// ----------------------------------------

typedef uint8_t  u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

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

typedef struct binary binary;

binary *new_binary(mempool *mp);
binary *new_binary_from_mem(mempool *mp, char *address, size_t size);
binary *new_binary_from_file(mempool *mp, str *filename);
binary *new_binary_with_zeros(mempool *mp, size_t size);

size_t  binary_length(binary *b);
void    binary_clear(binary *b);
int     binary_compare(binary *b1, binary *b2);
void    binary_cat(binary *b, binary *other);
binary *binary_clone(binary *b, mempool *mp);
void    binary_pad(binary *b, char value, size_t target_len);
void    binary_print_hex(binary *b, FILE *f);
// these manipulate the read/write pointer, like a file
void    binary_seek(binary *b, size_t offset); // emulate 
size_t  binary_tell(binary *b);
// all "read" funcs work at current offset, they advance offset
u8      binary_read_byte(binary *b);
u16     binary_read_word(binary *b);
u32     binary_read_dword(binary *b);
u64     binary_read_qword(binary *b);
void    binary_read_mem(binary *b, void *ptr, size_t length);
// all "write" funcs work at current offset, they advance offset
void    binary_write_byte(binary *b, u8 value);
void    binary_write_word(binary *b, u16 value);
void    binary_write_dword(binary *b, u32 value);
void    binary_write_qword(binary *b, u64 value);
void    binary_write_mem(binary *b, void *ptr, size_t length);
void    binary_write_zeros(binary *b, size_t length);
// these implicitely append data at the end of the buffer
void    binary_add_byte(binary *b, u8 value);
void    binary_add_word(binary *b, u16 value);
void    binary_add_dword(binary *b, u32 value);
void    binary_add_qword(binary *b, u64 value);
void    binary_add_mem(binary *b, void *ptr, size_t length);
void    binary_add_zeros(binary *b, size_t length);

binary *binary_get_slice(binary *b, size_t offset, size_t size, mempool *mp);
bool binary_save_to_file(binary *b, str *filename);

// -------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests();
#endif

