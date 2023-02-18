#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define P(c)    (((c) > 32 && (c) < 127) ? (c) : '.')


void print_16_hex(void *address, int size) {
    char *p = address;
    while (size > 0) {
        printf("    %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            P(p[0]),
            P(p[1]),
            P(p[2]),
            P(p[3]),
            P(p[4]),
            P(p[5]),
            P(p[6]),
            P(p[7]),
            P(p[8]),
            P(p[9]),
            P(p[10]),
            P(p[11]),
            P(p[12]),
            P(p[13]),
            P(p[14]),
            P(p[15])
        );
        p += 16;
        size -= 16;
    }
}


void print_pretty(char *str) {
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
    printf("%s", buff);
    free(buff);
}

bool load_text(char *filemame, char **buffer) {

}

bool save_text(char *filename, char *buffer) { 

}
