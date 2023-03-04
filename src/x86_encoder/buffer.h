#pragma once
#include <stdint.h>


typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;


typedef struct buffer buffer;

struct buffer {
    char *data;
    int capacity;
    int length;
    void (*clear)(buffer *buff);
    void (*append)(buffer *buff, buffer *source);
    void (*add_byte)(buffer *buff, u8 value);
    void (*add_word)(buffer *buff, u16 value);
    void (*add_dword)(buffer *buff, u32 value);
    void (*add_quad)(buffer *buff, u64 value);
    void (*add_mem)(buffer *buff, void *mem, int len);
    void (*add_strz)(buffer *buff, char *strz);
    void (*fill)(buffer *buff, int target_length, u8 filler);
    void (*free)(buffer *buff);
};

buffer *new_buffer();
