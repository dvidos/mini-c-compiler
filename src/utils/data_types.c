#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "data_types.h"


// --------------------------------------------

typedef struct str str;

typedef struct str {
    char *buff;
    int length;
    int capacity;
    mempool *mempool;
} str;

str *new_str(mempool *mp, const char *strz) {
    if (strz == NULL)
        strz = "";
    
    str *s = mempool_alloc(mp, sizeof(str), "str");
    s->length = strlen(strz);
    s->capacity = s->length + 1;
    s->buff = mempool_alloc(mp, s->capacity, "str_buff");
    strcpy(s->buff, strz);
    s->mempool = mp;

    return s;
}

str *new_strf(mempool *mp, const char *fmt, ...);

str *new_str_random(mempool *mp, int min_len, int max_len) {
    static const char allowed[] = "abcdefghijklmnopqrstuvwxyz0123456789";

    str *s = mempool_alloc(mp, sizeof(str), "str");

    int len = min_len + rand() % (max_len - min_len + 1);
    s->capacity = len + 1;
    s->buff = mempool_alloc(mp, s->capacity, "str_buff");
    for (int i = 0; i < len; i++)
        s->buff[i] = allowed[rand() % (sizeof(allowed) - 1)];
    s->buff[len] = '\0';
    s->length = len;
    s->mempool = mp;

    return s;
}

static void _ensure_capacity(str *s, int needed_capacity) {
    if (s->capacity >= needed_capacity)
        return;
    
    // allocate exponentially larger chunks
    int new_capacity = s->capacity;
    while (new_capacity < needed_capacity)
        new_capacity *= 2;
    
    // emulate realloc
    char *new_buff = mempool_alloc(s->mempool, new_capacity, "more str.buff");
    strcpy(new_buff, s->buff);
    s->capacity = new_capacity;
    s->buff = new_buff;
}

int  str_length(str *s) {
    return s->length;
}

bool str_empty(str *s) {
    return s->length == 0;
}

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

void str_cat(str *s1, str *s2) {
    _ensure_capacity(s1, s1->length + s2->length + 1);
    strcat(s1->buff, s2->buff);
    s1->length += s2->length;
}

void str_cats(str *s1, char *s2) {
    if (s2 == NULL)
        return;
    
    _ensure_capacity(s1, s1->length + strlen(s2) + 1);
    strcat(s1->buff, s2);
    s1->length += strlen(s2);
}

int  str_cmp(str *s1, str *s2) {
    return strcmp(s1->buff, s2->buff);
}

int  str_cmps(str *s1, char *s2) {
    return strcmp(s1->buff, s2);
}

bool str_equals(str *s1, str *s2) {
    if (s1 == s2)
        return true;
    if (s1->length != s2->length)
        return false;
    if (strcmp(s1->buff, s2->buff) != 0)
        return false;
    
    return true;
}

static unsigned int simple_hash(const char *ptr, int len) {
    // from the e-book about the elf format
    // i think this is also UNIX traditional hash
    unsigned int hash = 0, nibble;
    while (len-- > 0) {
        hash = (hash << 4) + *ptr++;
        if (nibble = (hash & 0xf0000000))
            hash ^= (nibble >> 24);
        hash &= 0x0fffffff;
    }
    return hash;
}

static unsigned int murmur_hash(const char *ptr, int len) {
    // from https://github.com/aappleby/smhasher
    const unsigned int multiplier = 0xc6a4a793;
    unsigned int hash = (len * multiplier);

    const unsigned char *p = (const unsigned char *)ptr;
    while (len >= 4) {
        unsigned int word = *(unsigned int *)p;
        hash += word;
        hash *= multiplier;
        hash ^= hash >> 16;
        p += 4;
        len -= 4;
    }

    if (len > 2)
        hash += p[2] << 16;
    if (len > 1)
        hash += p[1] << 8;
    if (len > 0) {
        hash += p[0];
        hash *= multiplier;
        hash ^= hash >> 16;
    }

    hash *= multiplier;
    hash ^= hash >> 10;
    hash *= multiplier;
    hash ^= hash >> 17;

    return hash;
}

unsigned int str_hash(str *s) {
    return murmur_hash(s->buff, s->length);
}

llist *str_split(str *s, str *delimiter);
str *str_join(str *delimiter, llist *list);
str *str_clone(str *s);
iterator *str_char_iterator(str *s);
iterator *str_token_iterator(str *s, str *delimiters);
bool str_save_file(str *s, str *filename);
str *str_load_file(str *filename, mempool *mp);

const char *str_charptr(str *s) {
    return s->buff;
}

#ifdef INCLUDE_UNIT_TESTS
void str_unit_tests() {
    // test the following:
    // str *new_str(mempool *mp, const char *str);
    // str *new_strf(mempool *mp, const char *fmt, ...);
    // int  str_length(str *s);
    // bool str_empty(str *s);
    // bool str_starts_with(str *s, str *fragment);
    // bool str_ends_with(str *s, str *fragment);
    // bool str_contains(str *s, str *fragment);
    // str *str_left(str *s, int len);
    // str *str_right(str *s, int len);
    // str *str_substr(str *s, int start, int len);
    // str *str_toupper(str *s);
    // str *str_tolower(str *s);
    // char str_char_at(str *s, int index);
    // int  str_index_of(str *s, str *needle, int start);
    // int  str_last_index_of(str *s, str *needle, int start);
    // int  str_pos(str *s, char c, int start);
    // int  str_last_pos(str *s, char c, int start);
    // str *str_replace(str *s, str *needle, str *replacement);
    // str *str_replace_chars(str *s, char c, str *replacement);
    // str *str_trim(str *s, str *characters);
    // str *str_padr(str *s, int len, char c);
    // str *str_padl(str *s, int len, char c);
    // str *str_cat(str *s1, str *s2);
    // str *str_cat3(str *s1, str *s2, str *s3);
    // int  str_cmp(str *s1, str s2);
    // bool str_equals(str *s1, str *s2);
    // unsigned long str_hash(str *s);
    // llist *str_split(str *s, str *delimiter);
    // str *str_join(str *delimiter, llist *list);
    // str *str_clone(str *s);
    // iterator *str_char_iterator(str *s);
    // iterator *str_token_iterator(str *s, str *delimiters);
    // bool str_save_file(str *s, str *filename);
    // str *str_load_file(str *filename, mempool *mp);
}
#endif


// ------------------------------------------------

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

#ifdef INCLUDE_UNIT_TESTS
void binary_unit_tests() {
    // test the following:
    // binary *new_binary(mempool *mp);
    // int binary_length(binary *b);
    // int binary_compare(binary *b1, binary *b2);
    // void binary_fill(binary *b, char value, int len);
    // binary *binary_cat(binary *b, binary *other)
    // binary *binary_clone(binary *b);
    // binary *binary_extract(binary *b, int offset, int size);
    // bool buff_save_file(str *s, str *filename);
    // binary *buff_load_file(str *filename, mempool *mp);
}
#endif

// -------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests() {
    str_unit_tests();
    binary_unit_tests();
}
#endif
