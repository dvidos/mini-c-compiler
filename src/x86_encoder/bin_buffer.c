#include "bin_buffer.h"
#include <stdlib.h>
#include <string.h>


static void _clear(struct bin_buffer *buff);
static void _append(struct bin_buffer *buff, struct bin_buffer *source);
static void _add_byte(struct bin_buffer *buff, u8 value);
static void _add_word(struct bin_buffer *buff, u16 value);
static void _add_dword(struct bin_buffer *buff, u32 value);
static void _add_quad(struct bin_buffer *buff, u64 value);
static void _fill(struct bin_buffer *buff, int target_length, u8 filler);
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
    p->add_mem = _add_mem;
    p->add_strz = _add_strz;
    p->fill = _fill;
    p->free = _free;

    return p;
}



static void _clear(struct bin_buffer *buff) {
    buff->length = 0;
}

static void _ensure_enough_capacity(struct bin_buffer *buff, int space_needed) {
    if (buff->length + space_needed >= buff->capacity) {
        while (buff->length + space_needed >= buff->capacity)
            buff->capacity *= 2;

        buff->data = realloc(buff->data, buff->capacity);
    }
}


static void _append(struct bin_buffer *buff, struct bin_buffer *source) {
    _ensure_enough_capacity(buff, source->length);
    memcpy(buff->data + buff->length, source->data, source->length);
    buff->length += source->length;
}

static void _add_byte(struct bin_buffer *buff, u8 value) {
    _ensure_enough_capacity(buff, 1);
    buff->data[buff->length] = value;
    buff->length++;
}

static void _add_word(struct bin_buffer *buff, u16 value) {
    _ensure_enough_capacity(buff, 2);
    u16 *p = (u16 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_dword(struct bin_buffer *buff, u32 value) {
    _ensure_enough_capacity(buff, 4);
    u32 *p = (u32 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _add_quad(struct bin_buffer *buff, u64 value) {
    _ensure_enough_capacity(buff, 4);
    u64 *p = (u64 *)(&buff->data[buff->length]);
    *p = value;
    buff->length += sizeof(value);
}

static void _fill(struct bin_buffer *buff, int target_length, u8 filler) {
    if (target_length <= buff->length)
        return;

    _ensure_enough_capacity(buff, target_length);
    int expansion = target_length - buff->length;
    memset(buff->data + buff->length, filler, expansion);
    buff->length += expansion;
}

static void _add_mem(struct bin_buffer *buff, void *mem, int len) {
    _ensure_enough_capacity(buff, len);
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

