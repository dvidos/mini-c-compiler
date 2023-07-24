#include <stdbool.h>
#include <stdio.h>


void print_pretty(const char *str, FILE *stream);


unsigned long round_up(unsigned long value, unsigned threshold);
char *set_extension(const char *path, char *extension);
unsigned long hash(const char *str);
