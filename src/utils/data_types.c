#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
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

str *new_str_from_mem(mempool *mp, const char *ptr, int length) {
    if (ptr == NULL)
        return new_str(mp, NULL);
    
    str *s = mempool_alloc(mp, sizeof(str), "str");
    s->capacity = length + 1;
    s->buff = mempool_alloc(mp, s->capacity, "str_buff");
    s->length = length;
    memcpy(s->buff, ptr, length);
    s->buff[s->length] = '\0';
    s->mempool = mp;

    return s;
}

str *new_strf(mempool *mp, const char *format, ...) {
    mempool *scratch = new_mempool();
    int buff_size = 128; // for start
    char *buff;
    va_list args;

    while (true) {
        buff = mempool_alloc(scratch, buff_size, "new_strf buff");
        memset(buff, 0, sizeof(buff));

        va_start(args, format);
        vsnprintf(buff, buff_size - 1, format, args);
        va_end(args);

        // if buffer was sufficiently big, we are done here!
        if (strlen(buff) < buff_size - 2)
            break;
        
        // buffer was exhausted, we need a bigger buffer
        buff_size *= 2;
    }

    str *final = new_str(mp, buff);
    mempool_release(scratch);

    return final;
}

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

int  str_len(str *s) {
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

bool str_starts_with(str *s, str *fragment) {
    return memcmp(s->buff, fragment->buff, fragment->length) == 0;
}

bool str_ends_with(str *s, str *fragment) {
    return memcmp(s->buff + s->length - fragment->length, fragment->buff, fragment->length) == 0;
}

bool str_contains(str *s, str *fragment) {
    return strstr(s->buff, fragment->buff) != NULL;
}

bool str_contains_char(str *s, char c) {
    return strchr(s->buff, c) != NULL;
}

str *str_left(str *s, int len) {
    if (len > s->length)
        len = s->length;
    
    str *other = new_str(s->mempool, NULL);
    str_ensure_capacity(other, len + 1);

    memcpy(other->buff, s->buff, len);
    other->buff[len] = 0;
    other->length = len;

    return other;
}

str *str_right(str *s, int len) {
    if (len > s->length)
        len = s->length;
    
    str *other = new_str(s->mempool, NULL);
    str_ensure_capacity(other, len + 1);

    memcpy(other->buff, s->buff + s->length - len, len);
    other->buff[len] = 0;
    other->length = len;
    
    return other;
}

str *str_substr(str *s, int start, int len) {
    // negative length means actual length minus it (e.g. -2 on a 10 chars string will yield 8)
    if (len < 0) 
        len = s->length + len; // remember it's negative
    // negative offset means count from end (e.g. -2 on a 10 chars string means start at 8)
    if (start < 0)
        start = s->length + start; // remember it's negative
    if (start + len > s->length)
        len = s->length - start;

    str *other = new_str(s->mempool, NULL);
    str_ensure_capacity(other, len + 1);

    memcpy(other->buff, s->buff + start, len);
    other->buff[len] = 0;
    other->length = len;
    
    return other;
}

str *str_toupper(str *s) {
    str *other = new_str(s->mempool, NULL);
    str_ensure_capacity(other, s->length + 1);

    for (int i = 0; i < s->length; i++)
        other->buff[i] = toupper(s->buff[i]);
    other->buff[s->length] = '\0';
    other->length = s->length;

    return other;
}

str *str_tolower(str *s) {
    str *other = new_str(s->mempool, NULL);
    str_ensure_capacity(other, s->length + 1);

    for (int i = 0; i < s->length; i++)
        other->buff[i] = tolower(s->buff[i]);
    other->buff[s->length] = '\0';
    other->length = s->length;

    return other;
}

char str_char_at(str *s, int index) {
    if (index < 0 || index >= s->length)
        return 0;
    
    return s->buff[index];
}

int str_index_of(str *s, str *needle, int start) {
    // very naive for now
    for (int i = start; i < s->length - needle->length; i++) {
        if (memcmp(s->buff + i, needle->buff, needle->length) == 0)
            return i;
    }
    return -1;
}

int str_index_of_any(str *s, str *characters, int start) {
    for (int i = start; i < s->length; i++) {
        if (str_contains_char(characters, s->buff[i]))
            return i;
    }
    return -1;
}

int str_last_index_of(str *s, str *needle) {
    // very naive for now
    for (int i = s->length - needle->length; i >= 0; i--) {
        if (memcmp(s->buff + i, needle->buff, needle->length) == 0)
            return i;
    }
    return -1;
}

int str_char_pos(str *s, char c, int start) {
    for (int i = start; i < s->length; i++) {
        if (s->buff[i] == c)
            return i;
    }
    return -1;
}

int str_last_char_pos(str *s, char c) {
    for (int i = s->length - 1; i >= 0; i--) {
        if (s->buff[i] == c)
            return i;
    }
    return -1;
}

str *str_replace(str *s, str *needle, str *replacement) {
    str *other = new_str(s->mempool, NULL);
    int pos = 0;
    while (pos < s->length) {
        if (memcmp(s->buff + pos, needle->buff, needle->length) == 0) {
            str_cat(other, replacement);
            pos += needle->length;
        } else {
            str_catc(other, s->buff[pos]);
            pos += 1;
        }
    }

    return other;
}

str *str_replace_chars(str *s, str *characters, str *replacements) {
    char small_str[2];
    str *other = new_str(s->mempool, NULL);
    int pos = 0;
    while (pos < s->length) {
        char c = s->buff[pos];
        int index = str_char_pos(characters, c, 0);
        if (index >= 0) {
            str_catc(other, str_char_at(replacements, index));
        } else {
            str_catc(other, c);
        }
        pos += 1;
    }

    return other;
}

str *str_trim(str *s, str *characters) {
    // do we maintain immutability, or do we change in-place?
    str *other = new_str(s->mempool, s->buff);

    while (other->length > 0 && str_contains_char(characters, other->buff[0])) {
        memmove(other->buff, other->buff + 1, other->length - 1 + 1);
        other->length -= 1;
    }

    while (other->length > 0 && str_contains_char(characters, other->buff[other->length - 1])) {
        other->buff[other->length - 1] = 0;
        other->length -= 1;
    }

    other->buff[other->length] = '\0';
    return other;
}

str *str_padr(str *s, int len, char c) {
    str *other = new_str(s->mempool, s->buff);
    str_ensure_capacity(other, len + 1);

    while (other->length < len) {
        other->buff[other->length] = c;
        other->length += 1;
    }

    return other;
}

str *str_padl(str *s, int len, char c) {
    str *other = new_str(s->mempool, s->buff);
    str_ensure_capacity(other, len + 1);

    while (other->length < len) {
        memmove(other->buff + 1, other->buff, other->length - 1 + 1);
        other->buff[0] = c;
        other->length += 1;
    }

    return other;
}

void str_cpy(str *s1, str *s2) {
    str_clear(s1);
    str_cat(s1, s2);
}

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

void str_catc(str *s, char c) {
    if (c == '\0')
        return;
    
    str_ensure_capacity(s, s->length + 1);
    s->buff[s->length] = c;
    s->length += 1;
    s->buff[s->length] = '\0';
}

int str_cmp(str *s1, str *s2) {
    return strcmp(s1->buff, s2->buff);
}

int str_cmps(str *s1, char *s2) {
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

llist *str_split(str *s, str *delimiters, bool include_empty_tokens, int max_items, mempool *mp) {
    // empty strings should be left out.
    llist *l = new_llist(mp);

    int start = 0;
    while (start < s->length && (max_items == -1 || llist_length(l) < max_items - 1)) {

        // find end delimiter
        int end = str_index_of_any(s, delimiters, start);
        if (end == -1)
            end = s->length;
        
        // should we include empty tokens?
        if ((end == start) && !include_empty_tokens) {
            start = end + 1;
            continue;
        }
        
        // add to list (using the new memory pool)
        str *item = new_str(mp, NULL);
        str_ensure_capacity(item, end - start + 1);
        memcpy(item->buff, s->buff + start, end - start);
        item->buff[end - start] = '\0';
        item->length = end - start;
        llist_add(l, item);

        // move on
        start = end + 1;
    }

    // there may be a limit to the number of tokens, add the remainder to the list
    if (start < str_len(s))
        llist_add(l, str_substr(s, start, str_len(s) - start));

    return l;
}

str *str_join(llist *strings, str *delimiter, mempool *mp) {
    mempool *scratch = new_mempool();
    iterator *it = llist_create_iterator(strings, scratch);
    
    int total_len = 0;
    for_iterator(str, s, it)
        total_len += str_len(s) + str_len(delimiter);

    str *joined = new_str(mp, NULL);
    str_ensure_capacity(joined, total_len + 1);

    bool first = true;
    for_iterator(str, s, it) {
        if (!first)
            str_cat(joined, delimiter);
        str_cat(joined, s);
        first = false;
    }

    mempool_release(scratch);
    return joined;
}

str *str_clone(str *s) {
    return new_str(s->mempool, s->buff);
}

bool str_save_file(str *s, str *filename) {
    FILE *f = fopen(str_charptr(filename), "w");
    if (f == NULL)
        return false;

    fwrite(s->buff, 1, s->length, f);
    fclose(f);
    return true;
}

str *str_load_file(str *filename, mempool *mp) {
    FILE *f = fopen(str_charptr(filename), "r");
    if (f == NULL)
        return NULL;
    
    fseek(f, 0, SEEK_END);
    long file_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    str *s = new_str(mp, NULL);
    str_ensure_capacity(s, file_size);

    fread(s->buff, 1, file_size, f);
    s->buff[file_size] = 0;
    s->length = file_size;

    fclose(f);
    return s;
}

const char *str_charptr(str *s) {
    return s->buff;
}

#ifdef INCLUDE_UNIT_TESTS
void str_unit_tests() {
    mempool *mp = new_mempool();
    str *s;
    str *s1;
    llist *l;

    // empty constructor
    s = new_str(mp, NULL);
    assert(s != NULL);
    assert(str_len(s) == 0); // some blackbox testing
    assert(str_is_empty(s));
    assert(s->capacity > 0); // some whitebox testing as well!
    assert(s->buff != NULL);

    // strz constructor
    s = new_str(mp, "testing");
    assert(s != NULL);
    assert(str_len(s) == 7); // some blackbox testing
    assert(!str_is_empty(s));
    assert(strcmp(str_charptr(s), "testing") == 0);

    // simple formatted constructor
    s = new_strf(mp, "%d", 123);
    assert(s != NULL);
    assert(str_len(s) == 3);
    assert(!str_is_empty(s));
    assert(strcmp(str_charptr(s), "123") == 0);

    // large buffer formatted constructor
    s = new_strf(mp, "%01024d", "1");
    assert(s != NULL);
    assert(str_len(s) == 1024);
    assert(!str_is_empty(s));

    // random constructor
    s = new_str_random(mp, 10, 20);
    assert(s != NULL);
    assert(str_len(s) >= 10);
    assert(str_len(s) <= 20);

    // clear()
    s = new_str(mp, "testing");
    assert(str_len(s)  == 7);
    assert(strcmp(str_charptr(s), "testing") == 0);
    str_clear(s);
    assert(str_len(s) == 0);
    assert(strcmp(str_charptr(s), "") == 0);

    // starts with, ends with, contains, contains char
    s = new_str(mp, "mary had a little lamb");
    assert(str_starts_with(s, new_str(mp, "m")));
    assert(str_starts_with(s, new_str(mp, "mary")));
    assert(str_starts_with(s, new_str(mp, "")));
    assert(!str_starts_with(s, new_str(mp, "a")));
    assert(str_ends_with(s, new_str(mp, "b")));
    assert(str_ends_with(s, new_str(mp, "lamb")));
    assert(str_ends_with(s, new_str(mp, "")));
    assert(!str_ends_with(s, new_str(mp, "a")));
    assert(str_contains(s, new_str(mp, "little")));
    assert(str_contains(s, new_str(mp, "")));
    assert(!str_contains(s, new_str(mp, "lot")));
    assert(str_contains_char(s, 'a'));
    assert(str_contains_char(s, 'l'));
    assert(str_contains_char(s, ' '));
    assert(!str_contains_char(s, 'A'));
    assert(!str_contains_char(s, 'z'));

    // str_left
    s = new_str(mp, "mary had a little lamb");
    s1 = str_left(s, 4);
    assert(s1 != NULL);
    assert(s1 != s);
    assert(str_len(s1) == 4);
    assert(s1->buff != s->buff);
    assert(strcmp(s1->buff, "mary") == 0);
    s1 = str_left(s, 0);
    assert(str_len(s1) == 0);
    s1 = str_left(s, 1000);
    assert(str_len(s1) == str_len(s));

    // str_right
    s = new_str(mp, "mary had a little lamb");
    s1 = str_right(s, 4);
    assert(s1 != NULL);
    assert(s1 != s);
    assert(str_len(s1) == 4);
    assert(s1->buff != s->buff);
    assert(strcmp(s1->buff, "lamb") == 0);
    s1 = str_right(s, 0);
    assert(str_len(s1) == 0);
    s1 = str_right(s, 1000);
    assert(str_len(s1) == str_len(s));

    // str_substr
    s = new_str(mp, "mary had a little lamb");
    s1 = str_substr(s, 1, 3);
    assert(strcmp(s1->buff, "ary") == 0);
    s1 = str_substr(s, -3, 3);
    assert(strcmp(s1->buff, "amb") == 0);
    s1 = str_substr(s, 2, -10);
    assert(strcmp(s1->buff, "ry had a lit") == 0);
    s1 = str_substr(s, 0, 0);
    assert(strcmp(s1->buff, "") == 0);

    // tolower / toupper
    s = new_str(mp, "Mary had a Little Lamb");
    s1 = str_tolower(s);
    assert(s1 != s);
    assert(strcmp(s1->buff, "mary had a little lamb") == 0);
    s1 = str_toupper(s);
    assert(s1 != s);
    assert(strcmp(s1->buff, "MARY HAD A LITTLE LAMB") == 0);

    // char_at, char_pos, last_char_pos
    s = new_str(mp, "mary had a little lamb");
    assert(str_char_at(s, 1) == 'a');
    assert(str_char_at(s, 21) == 'b');
    assert(str_char_at(s, 1024) == '\0');
    assert(str_char_pos(s, 'a', 0) == 1);
    assert(str_char_pos(s, 'a', 3) == 6);
    assert(str_char_pos(s, 'm', 0) == 0);
    assert(str_char_pos(s, 'z', 0) == -1);
    assert(str_last_char_pos(s, 'a') == 19);
    assert(str_last_char_pos(s, 'm') == 20);
    assert(str_last_char_pos(s, 'z') == -1);
    
    // index_of, last_index_of, index_of_any
    s = new_str(mp, "mary had a little lamb");
    assert(str_index_of(s, new_str(mp, "a"), 0) == 1);
    assert(str_index_of(s, new_str(mp, "a"), 3) == 6);
    assert(str_index_of(s, new_str(mp, "ab"), 0) == -1);
    assert(str_index_of(s, new_str(mp, "labmorgini"), 0) == -1);
    assert(str_index_of(s, new_str(mp, "m"), 0) == 0);
    assert(str_index_of(s, new_str(mp, ""), 0) == 0);
    assert(str_last_index_of(s, new_str(mp, "a")) == 19);
    assert(str_last_index_of(s, new_str(mp, "m")) == 20);
    assert(str_last_index_of(s, new_str(mp, "labmorgini")) == -1);
    assert(str_last_index_of(s, new_str(mp, "")) == str_len(s));
    assert(str_index_of_any(s, new_str(mp, "pqrs"), 0) == 2);
    assert(str_index_of_any(s, new_str(mp, "abc"), 0) == 1);
    assert(str_index_of_any(s, new_str(mp, "gk"), 0) == -1);
    assert(str_index_of_any(s, new_str(mp, ""), 0) == -1);

    // replace, replace chars
    s = new_str(mp, "mary had a little lamb");
    s1 = str_replace(s, new_str(mp, "lamb"), new_str(mp, "pony")); // should work
    assert(strcmp(s1->buff, "mary had a little pony") == 0);
    s1 = str_replace(s, new_str(mp, "lamp"), new_str(mp, "lantern")); // lamp != lamb
    assert(strcmp(s1->buff, "mary had a little lamb") == 0);
    s1 = str_replace_chars(s, new_str(mp, "altb"), new_str(mp, "41#6"));
    assert(strcmp(s1->buff, "m4ry h4d 4 1i##1e 14m6") == 0);

    // trim
    s = new_str(mp, "/path/to/directory/");
    s1 = str_trim(s, new_str(mp, "/"));
    assert(strcmp(s1->buff, "path/to/directory") == 0);
    s1 = str_trim(s, new_str(mp, "/py"));
    assert(strcmp(s1->buff, "ath/to/director") == 0);
    s1 = str_trim(s, new_str(mp, " \n\t"));
    assert(strcmp(s1->buff, "/path/to/directory/") == 0);

    // padr / padl
    s = new_str(mp, "123");
    s1 = str_padr(s, 1, '-');
    assert(strcmp(s1->buff, "123") == 0);
    s1 = str_padr(s, 6, '-');
    assert(strcmp(s1->buff, "123---") == 0);
    s1 = str_padl(s, 1, '-');
    assert(strcmp(s1->buff, "123") == 0);
    s1 = str_padl(s, 6, '-');
    assert(strcmp(s1->buff, "---123") == 0);

    // cpy
    s = new_str(mp, "test");
    assert(strcmp(s->buff, "test") == 0);
    str_cpy(s, new_str(mp, "other"));
    assert(strcmp(s->buff, "other") == 0);

    // cat, cats, catc
    s = new_str(mp, "string");
    str_cat(s, new_str(mp, "ify"));
    assert(strcmp(s->buff, "stringify") == 0);
    s = new_str(mp, "string");
    str_cats(s, "ed");
    assert(strcmp(s->buff, "stringed") == 0);
    s = new_str(mp, "string");
    str_catc(s, 's');
    assert(strcmp(s->buff, "strings") == 0);

    // cmp, cmps, equals
    s = new_str(mp, "string");
    s1 = new_str(mp, "string");
    assert(s != s1);
    assert(str_cmp(s1, s) == 0);
    assert(str_cmps(s1, "string") == 0);
    assert(str_equals(s1, s));
    s1 = new_str(mp, "integer");
    assert(s != s1);
    assert(str_cmp(s1, s) != 0);
    assert(str_cmps(s1, "string") != 0);
    assert(!str_equals(s1, s));

    // hash
    s = new_str(mp, "this is a string");
    unsigned long h = str_hash(s);
    assert(h != str_hash(new_str(mp, "this is a string."))); // dot
    assert(h != str_hash(new_str(mp, "This is a string")));  // capital
    assert(h == str_hash(new_str(mp, "this is a string")));  // same

    // split
    s = new_str(mp, "one,two||four");
    l = str_split(s, new_str(mp, "|,"), true, -1, mp); // base case
    assert(llist_length(l) == 4);
    assert(str_cmps(llist_get(l, 0), "one") == 0);
    assert(str_cmps(llist_get(l, 1), "two") == 0);
    assert(str_cmps(llist_get(l, 2), "") == 0);
    assert(str_cmps(llist_get(l, 3), "four") == 0);
    l = str_split(s, new_str(mp, "|,"), false, -1, mp); // skip empty tokens
    assert(llist_length(l) == 3);
    assert(str_cmps(llist_get(l, 0), "one") == 0);
    assert(str_cmps(llist_get(l, 1), "two") == 0);
    assert(str_cmps(llist_get(l, 2), "four") == 0);
    l = str_split(s, new_str(mp, "|,"), false, 2, mp); // max items to split
    assert(llist_length(l) == 2);
    assert(str_cmps(llist_get(l, 0), "one") == 0);
    assert(str_cmps(llist_get(l, 1), "two||four") == 0);

    // join
    l = new_llist(mp);
    llist_add(l, new_str(mp, "one"));
    llist_add(l, new_str(mp, "two"));
    llist_add(l, new_str(mp, "three"));
    s = str_join(l, new_str(mp, " & "), mp);  // base case
    assert(strcmp(s->buff, "one & two & three") == 0);
    llist_clear(l);
    s = str_join(l, new_str(mp, " & "), mp);  // behavior without items
    assert(strcmp(s->buff, "") == 0);

    // clone
    s = new_str(mp, "testing, testing");
    s1 = str_clone(s);
    assert(s1 != s);
    assert(s1->buff != s->buff);
    assert(strcmp(s1->buff, s->buff) == 0);

    // save, load
    char *text = "i am a string, as long as can be, \n is there room in the file, for a string like me? \n";
    char *fname = mempool_alloc(mp, 50, "fname");
    strcpy(fname, "/tmp/temp_XXXXXX");
    mkstemp(fname);
    s = new_str(mp, text);
    bool saved = str_save_file(s, new_str(mp, fname));
    assert(saved);
    // one hour later...
    str_clear(s);
    s = str_load_file(new_str(mp, fname), mp);
    assert(s != NULL);
    assert(strcmp(s->buff, text) == 0);
    unlink(fname);

    // mempool_print_allocations(mp, stdout);
    mempool_release(mp);
}
#endif


// ------------------------------------------------

typedef struct bin {
    char *buffer;
    size_t capacity;
    size_t length;
    size_t position;
    mempool *mempool;
} bin;

bin *new_bin(mempool *mp) {
    bin *b = mempool_alloc(mp, sizeof(bin), "binary");
    memset(b, 0, sizeof(bin));

    b->capacity = 16;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
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
    b->buffer = mempool_alloc(b->mempool, b->capacity, "binary buffer");
    memcpy(b->buffer, old_buffer, b->length);
}

bin *new_bin_from_mem(mempool *mp, char *address, size_t size) {
    bin *b = mempool_alloc(mp, sizeof(bin), "binary");
    memset(b, 0, sizeof(bin));

    b->capacity = size;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
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

    bin *b = mempool_alloc(mp, sizeof(bin), "binary");
    memset(b, 0, sizeof(bin));

    b->capacity = file_length;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");

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
    bin *b = mempool_alloc(mp, sizeof(bin), "binary");
    memset(b, 0, sizeof(bin));

    b->capacity = size;
    b->buffer = mempool_alloc(mp, b->capacity, "binary buffer");
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


#ifdef INCLUDE_UNIT_TESTS
void binary_unit_tests() {
    mempool *mp = new_mempool();
    bin *b;
    bin *b1;
    str *s;

    char *bytes32 = mempool_alloc(mp, 32, "bytes32");
    for (int i = 0; i < 32; i++)
        bytes32[i] = (char)i;

    char *mem = mempool_alloc(mp, 16, "mem");

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
    char *fname = mempool_alloc(mp, 50, "fname");
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

// -------------------------------------

#ifdef INCLUDE_UNIT_TESTS
void all_data_types_unit_tests() {
    str_unit_tests();
    binary_unit_tests();
}
#endif
