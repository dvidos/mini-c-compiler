#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define printable(c)    (((c) > 32 && (c) < 127) ? (c) : '.')


void print_pretty(const char *str, FILE *stream) {
    char *buff = malloc(strlen(str) * 2);
    char *dest = buff;
    
    while (*str != 0) {
        if      (*str == '\n') { *dest++ = '\\'; *dest++ = 'n'; }
        else if (*str == '\r') { *dest++ = '\\'; *dest++ = 'r'; }
        else if (*str == '\t') { *dest++ = '\\'; *dest++ = 't'; }
        else if (*str == '\\') { *dest++ = '\\'; *dest++ = '\\'; }
        else { *dest++ = *str; }
        str++;
    }
    *dest = *str; // zero terminator
    fprintf(stream, "%s", buff);
    free(buff);
}

unsigned long round_up(unsigned long value, unsigned threshold) {
    return (((value + threshold - 1) / threshold) * threshold);
}

char *set_extension(const char *path, char *extension) {

    char *last_dot = strrchr(path, '.');
    int len = (last_dot == NULL) ? strlen(path) : (last_dot - path);

    char *str = malloc(len + 1 + strlen(extension) + 1);
    memcpy(str, path, len);
    str[len] = '\0';

    if (strlen(extension) > 0) {
        strcat(str, ".");
        strcat(str, extension);
    }

    return str;
}

unsigned long hash(const char *str) {
    unsigned long hash = 0, nibble;

    while (*str) {
        hash = (hash << 4) + *str++;
        if (nibble = (hash & 0xf0000000))
            hash ^= (nibble >> 24);
        hash &= 0x0fffffff;
    }
    return hash;
}