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

static void str_ensure_capacity(str *s, int needed_capacity) {
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

bool str_is_empty(str *s) {
    return s->length == 0;
}

void str_clear(str *s) {
    str_ensure_capacity(s, 1);
    s->length = 0;
    s->buff[0] = 0;
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
    str_ensure_capacity(s1, s1->length + s2->length + 1);
    strcat(s1->buff, s2->buff);
    s1->length += s2->length;
}

void str_cats(str *s1, char *s2) {
    if (s2 == NULL)
        return;
    
    str_ensure_capacity(s1, s1->length + strlen(s2) + 1);
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
    // bool str_is_empty(str *s);
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

typedef struct binary {
    char *buffer;
    size_t capacity;
    size_t length;
    size_t position;
    mempool *mempool;
} binary;

binary *new_binary(mempool *mp) {
    binary *b = mempool_alloc(mp, sizeof(binary), "binary");
    memset(b, 0, sizeof(binary));

    b->capacity = 16;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
    b->length = 0;
    b->position = 0;
    b->mempool = mp;

    return b;
}

binary *new_binary_from_mem(mempool *mp, char *address, size_t size) {
    binary *b = mempool_alloc(mp, sizeof(binary), "binary");
    memset(b, 0, sizeof(binary));

    b->capacity = size;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
    memcpy(b->buffer, address, size);
    b->length = size;
    b->position = 0;
    b->mempool = mp;

    return b;
}

binary *new_binary_from_file(mempool *mp, str *filename) {
    FILE *f = fopen(str_charptr(filename), "rb");
    if (f == NULL)
        return NULL;
    fseek(f, 0, SEEK_END);
    size_t file_length = ftell(f);
    fseek(f, 0, SEEK_SET);

    binary *b = mempool_alloc(mp, sizeof(binary), "binary");
    memset(b, 0, sizeof(binary));

    b->capacity = file_length;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");

    fread(b->buffer, 1, file_length, f);
    fclose(f);

    b->length = file_length;
    b->position = 0;
    b->mempool = mp;

    return b;
}

binary *new_binary_with_zeros(mempool *mp, size_t size) {
    binary *b = mempool_alloc(mp, sizeof(binary), "binary");
    memset(b, 0, sizeof(binary));

    b->capacity = size;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
    memset(b->buffer, 0, size);
    b->length = size;
    b->position = 0;
    b->mempool = mp;

    return b;
}

size_t binary_length(binary *b) {
    return b->length;
}

void binary_clear(binary *b) {
    b->length = 0;
    b->position = 0;
}

int binary_compare(binary *b1, binary *b2) {
    if (b1->length != b2->length)
        return b2->length - b1->length;
    
    return memcmp(b1->buffer, b2->buffer, b1->length);
}

static void binary_ensure_capacity(binary *b, size_t capacity) {
    if (b->capacity >= capacity)
        return;
    
    while (b->capacity < capacity)
        b->capacity *= 2;
    
    char *old_buffer = b->buffer;
    b->buffer = mempool_alloc(b->mempool, b->capacity, "binary buffer");
    memcpy(b->buffer, old_buffer, b->length);
}

void binary_cat(binary *b, binary *other) {
    binary_ensure_capacity(b, b->length + other->length);
    memcpy(b->buffer + b->length, other->buffer, other->length);
    b->length += other->length;
}

binary *binary_clone(binary *b, mempool *mp) {
    return new_binary_from_mem(mp, b->buffer, b->length);
}

void binary_pad(binary *b, char value, size_t target_len) {
    if (b->length >= target_len)
        return;
    
    binary_ensure_capacity(b, target_len);
    int gap = target_len - b->length;
    memset(b->buffer + b->length, 0, gap);
    b->length += gap;
}

void binary_print_hex(binary *b, FILE *f);

// emulate a file a bit? i think it's useful to maintain internal pointer
void binary_seek(binary *b, size_t offset) {
    if (offset > b->length)
        offset = b->length;
    b->position = offset;
}

// all "read" funcs work at current offset, they advance offset
u8 binary_read_byte(binary *b) {
    if (b->position >= b->length)
        return 0;
    
    u8 value = *(u8 *)(b->buffer + b->position);
    b->position += 1;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u16  binary_read_word(binary *b) {
    if (b->position >= b->length)
        return 0;
    
    u16 value = *(u16 *)(b->buffer + b->position);
    b->position += 2;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u32  binary_read_dword(binary *b) {
    if (b->position >= b->length)
        return 0;
    
    u32 value = *(u32 *)(b->buffer + b->position);
    b->position += 4;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

u64  binary_read_qword(binary *b) {
    if (b->position >= b->length)
        return 0;
    
    u64 value = *(u64 *)(b->buffer + b->position);
    b->position += 8;
    if (b->position > b->length)
        b->position = b->length;
    
    return value;
}

void binary_read_mem(binary *b, void *ptr, size_t length) {
    if (b->position >= b->length)
        return;
    
    memcpy(ptr, b->buffer + b->position, length);
    b->position += length;
    if (b->position > b->length)
        b->position = b->length;
}

// all "write" funcs work at current offset, they advance offset
void binary_write_byte(binary *b, u8 value) {
    binary_ensure_capacity(b, b->position + 1);
    b->buffer[b->position] = value;
    b->position += 1;
    if (b->position > b->length)
        b->length = b->position;
}

void binary_write_word(binary *b, u16 value) {
    binary_ensure_capacity(b, b->position + sizeof(u16));
    *(u16 *)(b->buffer + b->position) = value;
    b->position += sizeof(u16);
    if (b->position > b->length)
        b->length = b->position;
}

void binary_write_dword(binary *b, u32 value) {
    binary_ensure_capacity(b, b->position + sizeof(u32));
    *(u32 *)(b->buffer + b->position) = value;
    b->position += sizeof(u32);
    if (b->position > b->length)
        b->length = b->position;
}

void binary_write_qword(binary *b, u64 value) {
    binary_ensure_capacity(b, b->position + sizeof(u64));
    *(u64 *)(b->buffer + b->position) = value;
    b->position += sizeof(u64);
    if (b->position > b->length)
        b->length = b->position;
}

void binary_write_mem(binary *b, void *ptr, size_t length) {
    binary_ensure_capacity(b, b->position + length);
    memcpy(b->buffer + b->position, ptr, length);
    b->position += length;
    if (b->position > b->length)
        b->length = b->position;
}

void binary_write_zeros(binary *b, size_t length) {
    binary_ensure_capacity(b, b->position + length);
    memset(b->buffer + b->position, 0, length);
    b->position += length;
    if (b->position > b->length)
        b->length = b->position;
}

// these implicitely append data at the end of the buffer
void binary_add_byte(binary *b, u8 value) {
    b->position = b->length;
    binary_write_byte(b, value);
}

void binary_add_word(binary *b, u16 value)  {
    b->position = b->length;
    binary_write_word(b, value);
}

void binary_add_dword(binary *b, u32 value) {
    b->position = b->length;
    binary_write_dword(b, value);
}

void binary_add_qword(binary *b, u64 value) {
    b->position = b->length;
    binary_write_qword(b, value);
}

void binary_add_mem(binary *b, void *ptr, size_t length) {
    b->position = b->length;
    binary_write_mem(b, ptr, length);
}

void binary_add_zeros(binary *b, size_t length) {
    b->position = b->length;
    binary_write_zeros(b, length);
}

binary *binary_get_slice(binary *b, size_t offset, size_t size, mempool *mp) {
    return new_binary_from_mem(mp, b->buffer + offset, size);
}

bool binary_save_to_file(binary *b, str *filename) {
    FILE *f = fopen(str_charptr(filename), "wb");
    if (f == NULL)
        return false;

    fwrite(b->buffer, 1, b->length, f);
    fclose(f);
    return true;
}


#ifdef INCLUDE_UNIT_TESTS
void binary_unit_tests() {
    assert("binary unit tests must be created" == 0);


    // binary *new_binary(mempool *mp);
    // binary *new_binary_from_mem(mempool *mp, char *address, size_t size);
    // binary *new_binary_from_file(mempool *mp, str *filename);
    // binary *new_binary_with_zeros(mempool *mp, size_t size);

    // size_t  binary_length(binary *b);
    // void    binary_clear(binary *b);
    // int     binary_compare(binary *b1, binary *b2);
    // void    binary_cat(binary *b, binary *other);
    // binary *binary_clone(binary *b, mempool *mp);
    // void    binary_pad(binary *b, char value, size_t target_len);
    // void    binary_print_hex(binary *b, FILE *f);

    // // emulate a file a bit? i think it's useful to maintain internal pointer
    // void binary_seek(binary *b, size_t offset);

    // // all "read" funcs work at current offset, they advance offset
    // u8   binary_read_byte(binary *b);
    // u16  binary_read_word(binary *b);
    // u32  binary_read_dword(binary *b);
    // u64  binary_read_qword(binary *b);
    // void binary_read_mem(binary *b, void *ptr, size_t length);

    // // all "write" funcs work at current offset, they advance offset
    // void binary_write_byte(binary *b, u8 value);
    // void binary_write_word(binary *b, u16 value);
    // void binary_write_dword(binary *b, u32 value);
    // void binary_write_qword(binary *b, u64 value);
    // void binary_write_mem(binary *b, void *ptr, size_t length);
    // void binary_write_zeros(binary *b, size_t length);

    // // these implicitely append data at the end of the buffer
    // void binary_add_byte(binary *b, u8 value);
    // void binary_add_word(binary *b, u16 value);
    // void binary_add_dword(binary *b, u32 value);
    // void binary_add_qword(binary *b, u64 value);
    // void binary_add_mem(binary *b, void *ptr, size_t length);
    // void binary_add_zeros(binary *b, size_t length);

    // binary *binary_get_slice(binary *b, size_t offset, size_t size, mempool *mp);
    // bool binary_save_to_file(binary *b, str *filename);
}
#endif

// -------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests() {
    str_unit_tests();
    binary_unit_tests();
}
#endif
