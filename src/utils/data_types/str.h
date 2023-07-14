#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "../mempool.h"
#include "../unit_tests.h"


typedef struct str str;
typedef struct llist llist;


str *new_str(mempool *mp, const char *str);
str *new_str_from_mem(mempool *mp, const char *ptr, int length);
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
void str_cpy(str *s1, str *s2);
void str_cat(str *s1, str *s2);
void str_cats(str *s1, char *s2);
void str_catf(str *s, char *format, ...);
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
str *str_change_extension(str *filename, char *new_extension);
str *str_filename_only(str *path);

#ifdef INCLUDE_UNIT_TESTS
void str_unit_tests();
#endif
