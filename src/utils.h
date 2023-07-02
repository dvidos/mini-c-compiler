#include <stdbool.h>
#include <stdio.h>


void print_16_hex(void *buffer, int size, int indent);
void print_pretty(char *str, FILE *stream);


bool load_text(char *filemame, char **buffer);
bool save_text(char *filename, char *buffer);

unsigned long round_up(unsigned long value, unsigned threshold);
char *set_extension(const char *path, char *extension);

unsigned long hash(const char *str);