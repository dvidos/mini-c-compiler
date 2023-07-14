#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include "str.h"
#include "../data_structs/llist.h"


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
    
    str *s = mpalloc(mp, str);
    s->length = strlen(strz);
    s->capacity = s->length + 1;
    s->buff = mpallocn(mp, s->capacity, "str_buff");
    strcpy(s->buff, strz);
    s->mempool = mp;

    return s;
}

str *new_str_from_mem(mempool *mp, const char *ptr, int length) {
    if (ptr == NULL)
        return new_str(mp, NULL);
    
    str *s = mpalloc(mp, str);
    s->capacity = length + 1;
    s->buff = mpallocn(mp, s->capacity, "str_buff");
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
        buff = mpallocn(scratch, buff_size, "new_strf buff");
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

    str *s = mpalloc(mp, str);

    int len = min_len + rand() % (max_len - min_len + 1);
    s->capacity = len + 1;
    s->buff = mpallocn(mp, s->capacity, "str_buff");
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
    char *new_buff = mpallocn(s->mempool, new_capacity, "more str.buff");
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

void str_catf(str *s, char *format, ...) {
    mempool *scratch = new_mempool();
    int buff_size = 128; // for start
    char *buff;
    va_list args;

    while (true) {
        buff = mpallocn(scratch, buff_size, "new_strf buff");
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

    str_cats(s, buff);
    mempool_release(scratch);
}

void str_catc(str *s, char c) {
    if (c == '\0')
        return;
    
    str_ensure_capacity(s, s->length + 2);
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
    return s == NULL ? NULL : s->buff;
}

str *str_change_extension(str *filename, char *new_extension) {
    int pos = str_last_char_pos(filename, '.');
    if (pos == -1 && (new_extension == NULL || strlen(new_extension) == 0))
        return filename;

    str *result;
    if (pos == -1) {
        // no extension present, add new extension
        result = str_clone(filename);
        str_catc(result, '.');
        str_cat(result, new_str(filename->mempool, new_extension));
    } else {
        result = str_substr(filename, 0, pos);
        if (new_extension != NULL && strlen(new_extension) > 0) {
            str_catc(result, '.');
            str_cat(result, new_str(filename->mempool, new_extension));
        }
    }

    return result;
}

str *str_filename_only(str *path) {
    int pos = str_last_char_pos(path, '/');
    if (pos == -1)
        return path;

    return str_substr(path, pos + 1, 1000);
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
    char *fname = mpallocn(mp, 50, "fname");
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

