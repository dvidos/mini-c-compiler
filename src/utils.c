#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define printable(c)    (((c) > 32 && (c) < 127) ? (c) : '.')


void print_16_hex(void *address, int size) {
    unsigned char *p = address;
    while (size > 0) {
        printf("    %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
            p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15],
            printable(p[0]),
            printable(p[1]),
            printable(p[2]),
            printable(p[3]),
            printable(p[4]),
            printable(p[5]),
            printable(p[6]),
            printable(p[7]),
            printable(p[8]),
            printable(p[9]),
            printable(p[10]),
            printable(p[11]),
            printable(p[12]),
            printable(p[13]),
            printable(p[14]),
            printable(p[15])
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

bool load_text(char *filename, char **buffer) {

    FILE *f = fopen(filename, "r");
    if (f == NULL)
        return false;

    fseek(f, 0, SEEK_END);
    long size = (int)ftell(f);
    fseek(f, 0, SEEK_SET);

    // allow for null terminator
    char *p = malloc(size + 1);
    memset(p, 0, size + 1);

    long gotten = fread(p, 1, size, f);
    fclose(f);

    *buffer = p;
    return (gotten == size);
}

bool save_text(char *filename, char *buffer) { 

    FILE *f = fopen(filename, "w");
    if (f == NULL)
        return false;

    long len = strlen(buffer);
    long written = fwrite(buffer, 1, len, f);
    fclose(f);

    return (written == len);
}

unsigned long round_up(unsigned long value, unsigned threshold) {
    return (((value + threshold - 1) / threshold) * threshold);
}
