#include "bin_buffer.h"
#include <stdlib.h>
#include <string.h>


static void _clear(struct bin_buffer *buff);
static void _append(struct bin_buffer *buff, struct bin_buffer *source);
static void _add_byte(struct bin_buffer *buff, u8 value);
static void _add_word(struct bin_buffer *buff, u16 value);
static void _add_dword(struct bin_buffer *buff, u32 value);
static void _add_quad(struct bin_buffer *buff, u64 value);
static void _add_repeat_bytes(struct bin_buffer *buff, u8 value, int times);
static void _add_repeat_words(struct bin_buffer *buff, u16 value, int times);
static void _add_repeat_dwords(struct bin_buffer *buff, u32 value, int times);
static void _add_repeat_quads(struct bin_buffer *buff, u64 value, int times);
static void _add_mem(struct bin_buffer *buff, void *mem, int len);
static void _add_strz(struct bin_buffer *buff, char *strz);
static void _free(struct bin_buffer *buff);


struct bin_buffer *new_bin_buffer() {
    struct bin_buffer *p = malloc(sizeof(struct bin_buffer));
    p->capacity = 10;
    p->data = malloc(p->capacity);
    p->length = 0;

    p->clear = _clear;
    p->append = _append;
    p->add_byte = _add_byte;
    p->add_word = _add_word;
    p->add_dword = _add_dword;
    p->add_quad = _add_quad;
    p->add_repeat_bytes = _add_repeat_bytes;
    p->add_repeat_words = _add_repeat_words;
    p->add_repeat_dwords = _add_repeat_dwords;
    p->add_repeat_quads = _add_repeat_quads;
    p->add_strz = _add_strz;
    p->free = _free;

    return p;
}



static void _clear(struct bin_buffer *buff) {
    buff->length = 0;
}

static void _append(struct bin_buffer *buff, struct bin_buffer *source) {
    if (buff->length + source->length >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    memcpy(buff->data + buff->length, source->data, source->length);
    buff->length += source->length;
}

static void _add_byte(struct bin_buffer *buff, u8 value) {
    if (buff->length + 1 >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    buff->data[buff->length] = value;
    buff->length++;
}

static void _add_word(struct bin_buffer *buff, u16 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u16 *p = (u16 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_dword(struct bin_buffer *buff, u32 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u32 *p = (u32 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_quad(struct bin_buffer *buff, u64 value) {
    if (buff->length + sizeof(value) >= buff->capacity) {
        buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }
    u64 *p = (u64 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_repeat_bytes(struct bin_buffer *buff, u8 value, int times) {
    while (times--)
        _add_byte(buff, value);
}

static void _add_repeat_words(struct bin_buffer *buff, u16 value, int times) {
    while (times--)
        _add_word(buff, value);
}

static void _add_repeat_dwords(struct bin_buffer *buff, u32 value, int times) {
    while (times--)
        _add_dword(buff, value);
}

static void _add_repeat_quads(struct bin_buffer *buff, u64 value, int times) {
    while (times--)
        _add_quad(buff, value);
}

static void _add_mem(struct bin_buffer *buff, void *mem, int len) {
    if (buff->length + len >= buff->capacity) {
        while (buff->length + len < buff->capacity)
            buff->capacity *= 2;
        buff->data = realloc(buff->data, buff->capacity);
    }

    char *pos = &buff->data[buff->length];
    memcpy(pos, mem, len);
    buff->length += len;
}

static void _add_strz(struct bin_buffer *buff, char *strz) {
    _add_mem(buff, strz, strlen(strz) + 1); // include null terminator
}

static void _free(struct bin_buffer *buff) {
    free(buff->data);
    free(buff);
}

