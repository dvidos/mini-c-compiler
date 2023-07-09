#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>


#define printable(c)    (((c) > 32 && (c) < 127) ? (c) : '.')


void print_16_hex(void *buffer, int size, int indent) {
    unsigned long offset;
    unsigned char *p = buffer;
    char indentation[64];
    char prep[64];

    memset(indentation, ' ', sizeof(indentation));
    if (indent > sizeof(indentation) - 1)
        indent = sizeof(indentation) - 1;
    indentation[indent] = '\0';

    offset = 0;
    while (size > 0) {
        for (int i = 0; i < 16; i++) {
            if (i < size) {
                int c = (int)p[i];
                sprintf(&prep[i * 4], "%02x", c);
                prep[i*4  + 3] = (c >= ' ' && c <= '~') ? c : '.';
            } else {
                strcpy(&prep[i * 4], "  ");
                prep[i*4  + 3] = ' ';
            }
        }

        printf("%s%08lx   %2s %2s %2s %2s %2s %2s %2s %2s  %2s %2s %2s %2s %2s %2s %2s %2s   %c%c%c%c%c%c%c%c %c%c%c%c%c%c%c%c\n",
            indentation, offset,
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
        size -= 16;
        offset += 16;
    }
}


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