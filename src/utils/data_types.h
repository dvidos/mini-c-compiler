#pragma once
#include <stdbool.h>
#include "mempool.h"
#include "data_structs.h"
#include "../unit_tests.h"


// ----------------------------------------

typedef struct str str;
typedef struct llist llist;
typedef struct iterator iterator;

str *new_str(mempool *mp, const char *str);
str *new_strf(mempool *mp, const char *fmt, ...);
int  str_length(str *s);
bool str_empty(str *s);
bool str_starts_with(str *s, str *fragment);
bool str_ends_with(str *s, str *fragment);
bool str_contains(str *s, str *fragment);
str *str_left(str *s, int len);
str *str_right(str *s, int len);
str *str_substr(str *s, int start, int len);
str *str_toupper(str *s);
str *str_tolower(str *s);
char str_char_at(str *s, int index);
int  str_index_of(str *s, str *needle, int start);
int  str_last_index_of(str *s, str *needle, int start);
int  str_pos(str *s, char c, int start);
int  str_last_pos(str *s, char c, int start);
str *str_replace(str *s, str *needle, str *replacement);
str *str_replace_chars(str *s, char c, str *replacement);
str *str_trim(str *s, str *characters);
str *str_padr(str *s, int len, char c);
str *str_padl(str *s, int len, char c);
str *str_cat(str *s1, str *s2);
str *str_cat3(str *s1, str *s2, str *s3);
int  str_cmp(str *s1, str *s2);
bool str_equals(str *s1, str *s2);
unsigned long str_hash(str *s);
llist *str_split(str *s, str *delimiter);
str *str_join(str *delimiter, llist *list);
str *str_clone(str *s);
iterator *str_char_iterator(str *s);
iterator *str_token_iterator(str *s, str *delimiters);
bool str_save_file(str *s, str *filename);
str *str_load_file(str *filename, mempool *mp);


// -------------------------------------------

typedef struct binary binary;

binary *new_binary(mempool *mp);
int binary_length(binary *b);
int binary_compare(binary *b1, binary *b2);
void binary_fill(binary *b, char value, int len);
binary *binary_cat(binary *b, binary *other);
binary *binary_clone(binary *b);
binary *binary_extract(binary *b, int offset, int size);
bool buff_save_file(str *s, str *filename);
binary *buff_load_file(str *filename, mempool *mp);


// -------------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests();
#endif

